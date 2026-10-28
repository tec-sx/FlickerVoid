"""Client for the Unreal Editor's Python Remote Execution protocol (PythonScriptRemoteExecution.cpp).

Discovery and connection requests go over UDP multicast; the editor then connects to a TCP server
opened here, and commands and their results travel over that connection as JSON messages.
"""

import json
import socket
import time
import uuid
from dataclasses import dataclass

PROTOCOL_VERSION = 1
PROTOCOL_MAGIC = "ue_py"

TYPE_PING = "ping"
TYPE_PONG = "pong"
TYPE_OPEN_CONNECTION = "open_connection"
TYPE_CLOSE_CONNECTION = "close_connection"
TYPE_COMMAND = "command"
TYPE_COMMAND_RESULT = "command_result"

MODE_EXEC_FILE = "ExecuteFile"

_RECEIVE_SIZE = 65536


class RemoteError(RuntimeError):
    pass


@dataclass
class RemoteConfig:
    multicast_group: tuple = ("239.0.0.1", 6766)
    multicast_bind_address: str = "127.0.0.1"
    multicast_ttl: int = 0
    command_endpoint: tuple = ("127.0.0.1", 6776)
    discovery_timeout: float = 5.0
    command_timeout: float = 600.0


def encode_message(type_, source, dest=None, data=None):
    message = {"version": PROTOCOL_VERSION, "magic": PROTOCOL_MAGIC, "type": type_, "source": source}
    if dest:
        message["dest"] = dest
    if data:
        message["data"] = data
    return json.dumps(message, ensure_ascii=False).encode("utf-8")


def decode_message(payload, node_id):
    """The message as a dict, or None when it is malformed or not meant for this node."""
    try:
        message = json.loads(payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        return None
    if not isinstance(message, dict):
        return None
    if message.get("version") != PROTOCOL_VERSION or message.get("magic") != PROTOCOL_MAGIC:
        return None
    if message.get("source") == node_id:
        return None
    dest = message.get("dest")
    if dest and dest != node_id:
        return None
    return message


class RemoteSession:
    """One editor connection, used as a context manager: discover, connect, run commands, close."""

    def __init__(self, config=None, project_dir=None):
        self.config = config or RemoteConfig()
        self.project_dir = project_dir
        self.node_id = str(uuid.uuid4())
        self.remote_node = None
        self._udp = None
        self._listen = None
        self._command = None

    def __enter__(self):
        try:
            self._open_udp()
            self.remote_node = self._discover()
            self._connect()
        except BaseException:
            self.close()
            raise
        return self

    def __exit__(self, *_):
        self.close()

    def run(self, command, unattended=True):
        """Runs Python in the editor; returns the command_result data (success, result, output)."""
        self._command.sendall(encode_message(TYPE_COMMAND, self.node_id, self.remote_node["node_id"], {
            "command": command,
            "unattended": unattended,
            "exec_mode": MODE_EXEC_FILE,
        }))
        return self._receive_result()

    def close(self):
        if self._command:
            try:
                self._send_udp(TYPE_CLOSE_CONNECTION, self.remote_node["node_id"])
            except OSError:
                pass
        for sock in (self._command, self._listen, self._udp):
            if sock:
                try:
                    sock.close()
                except OSError:
                    pass
        self._command = self._listen = self._udp = None

    def _open_udp(self):
        group_host, group_port = self.config.multicast_group
        bind = self.config.multicast_bind_address
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.SOL_SOCKET, getattr(socket, "SO_REUSEPORT", socket.SO_REUSEADDR), 1)
        sock.bind((bind, group_port))
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, self.config.multicast_ttl)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(group_host) + socket.inet_aton(bind))
        sock.settimeout(0.2)
        self._udp = sock

    def _send_udp(self, type_, dest=None, data=None):
        self._udp.sendto(encode_message(type_, self.node_id, dest, data), self.config.multicast_group)

    def _discover(self):
        """Pings until an editor answers; prefers the one whose project is the configured project."""
        nodes = {}
        deadline = time.monotonic() + self.config.discovery_timeout
        next_ping = 0.0
        while time.monotonic() < deadline:
            if time.monotonic() >= next_ping:
                self._send_udp(TYPE_PING)
                next_ping = time.monotonic() + 1.0
            try:
                payload = self._udp.recv(_RECEIVE_SIZE)
            except socket.timeout:
                if nodes:
                    break
                continue
            message = decode_message(payload, self.node_id)
            if message and message.get("type") == TYPE_PONG:
                node = dict(message.get("data") or {})
                node["node_id"] = message["source"]
                nodes[node["node_id"]] = node
                if self._is_project(node):
                    return node

        if not nodes:
            raise RemoteError("No Unreal Editor answered. Is it running with Python Remote Execution enabled?")
        if self.project_dir:
            projects = ", ".join(sorted(node.get("project_name", "?") for node in nodes.values()))
            if len(nodes) > 1:
                raise RemoteError(f"Several editors answered ({projects}) and none has the configured project open")
        return next(iter(nodes.values()))

    def _is_project(self, node):
        if not self.project_dir:
            return False
        root = node.get("project_root", "")
        return _normalize(root) == _normalize(self.project_dir)

    def _connect(self):
        listen = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        listen.setsockopt(socket.SOL_SOCKET, getattr(socket, "SO_REUSEPORT", socket.SO_REUSEADDR), 1)
        listen.bind(self.config.command_endpoint)
        listen.listen(1)
        listen.settimeout(2.0)
        self._listen = listen

        host, port = self.config.command_endpoint
        for _ in range(5):
            self._send_udp(TYPE_OPEN_CONNECTION, self.remote_node["node_id"], {"command_ip": host, "command_port": port})
            try:
                self._command = listen.accept()[0]
                self._command.settimeout(self.config.command_timeout)
                return
            except socket.timeout:
                continue
        raise RemoteError("The Unreal Editor answered but did not open the command connection")

    def _receive_result(self):
        buffer = b""
        while True:
            try:
                chunk = self._command.recv(_RECEIVE_SIZE)
            except socket.timeout as error:
                raise RemoteError(f"No result from the Unreal Editor within {self.config.command_timeout:g} s") from error
            if not chunk:
                raise RemoteError("The Unreal Editor closed the connection")
            buffer += chunk
            try:
                json.loads(buffer.decode("utf-8"))
            except (UnicodeDecodeError, json.JSONDecodeError):
                continue
            message = decode_message(buffer, self.node_id)
            buffer = b""
            if message and message.get("type") == TYPE_COMMAND_RESULT:
                return message.get("data") or {}


def _normalize(path):
    return path.replace("\\", "/").rstrip("/").lower()
