import json
import socket
import threading
import unittest

import _path  # noqa: F401
from unreal_link import link, remote


class MessageTests(unittest.TestCase):
    def test_round_trip_and_filter(self):
        payload = remote.encode_message(remote.TYPE_PONG, "editor", "me", {"project_name": "FlickerVoid"})
        self.assertEqual(remote.decode_message(payload, "me")["data"]["project_name"], "FlickerVoid")
        self.assertIsNone(remote.decode_message(payload, "someone_else"))
        self.assertIsNone(remote.decode_message(remote.encode_message(remote.TYPE_PING, "me"), "me"))
        self.assertIsNone(remote.decode_message(b'{"version": 2}', "me"))


class ResultTests(unittest.TestCase):
    def test_marker(self):
        value = {"imported": ["/Game/A"]}
        result = {"success": True, "output": [{"type": "Info", "output": "noise"},
                                              {"type": "Info", "output": link.RESULT_MARKER + json.dumps({"ok": True, "value": value})}]}
        self.assertEqual(link.parse_result(result), value)

    def test_error(self):
        result = {"output": [{"type": "Info", "output": link.RESULT_MARKER + json.dumps({"ok": False, "error": "boom"})}]}
        with self.assertRaisesRegex(link.LinkError, "boom"):
            link.parse_result(result)

    def test_no_result(self):
        with self.assertRaisesRegex(link.LinkError, "SyntaxError"):
            link.parse_result({"success": False, "output": [{"type": "Error", "output": "SyntaxError"}]})

    def test_script_runs(self):
        script = link.build_script("editor_info", {"a": 1}).replace("import unreal", "")
        script = script.replace("def main(params):", "def main(params):\n    return params\ndef _unused(params):")
        scope = {}
        import contextlib
        import io
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            exec(script, scope)
        self.assertEqual(link.parse_result({"output": [{"type": "Info", "output": out.getvalue()}]}), {"a": 1})


class FakeEditor(threading.Thread):
    """Answers pings and runs commands by echoing them back, like the editor's protocol does."""

    def __init__(self, config, project_root):
        super().__init__(daemon=True)
        self.config = config
        self.project_root = project_root
        self.node_id = "fake-editor"
        group_host, group_port = config.multicast_group
        self.udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.udp.setsockopt(socket.SOL_SOCKET, getattr(socket, "SO_REUSEPORT", socket.SO_REUSEADDR), 1)
        self.udp.bind((config.multicast_bind_address, group_port))
        self.udp.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
        self.udp.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(group_host) + socket.inet_aton(config.multicast_bind_address))
        self.udp.settimeout(5)

    def run(self):
        while True:
            message = remote.decode_message(self.udp.recv(65536), self.node_id)
            if not message:
                continue
            if message["type"] == remote.TYPE_PING:
                self.udp.sendto(remote.encode_message(remote.TYPE_PONG, self.node_id, message["source"],
                                                      {"project_name": "FlickerVoid", "project_root": self.project_root}), self.config.multicast_group)
            elif message["type"] == remote.TYPE_OPEN_CONNECTION:
                data = message["data"]
                tcp = socket.create_connection((data["command_ip"], data["command_port"]))
                command = json.loads(tcp.recv(65536).decode())
                output = "x" * 200000 + "\n" + link.RESULT_MARKER + json.dumps({"ok": True, "value": len(command["data"]["command"])})
                tcp.sendall(remote.encode_message(remote.TYPE_COMMAND_RESULT, self.node_id, command["source"],
                                                  {"success": True, "output": [{"type": "Info", "output": output}]}))
                tcp.close()
                self.udp.close()
                return


class SessionTests(unittest.TestCase):
    def test_fake_editor(self):
        # Bound to all adapters: some containers have no multicast route on loopback.
        config = remote.RemoteConfig(multicast_group=("239.0.0.1", 16766), multicast_bind_address="0.0.0.0", command_endpoint=("127.0.0.1", 16776), discovery_timeout=3)
        try:
            editor = FakeEditor(config, "D:/Projects/FlickerVoid/")
        except OSError as error:
            self.skipTest(f"No multicast here: {error}")
        editor.start()
        try:
            with remote.RemoteSession(config, "d:\\Projects\\FlickerVoid") as session:
                self.assertEqual(session.remote_node["node_id"], "fake-editor")
                self.assertEqual(link.parse_result(session.run("print(1)")), len("print(1)"))
        except remote.RemoteError as error:
            self.skipTest(f"Multicast loopback unavailable: {error}")


if __name__ == "__main__":
    unittest.main()
