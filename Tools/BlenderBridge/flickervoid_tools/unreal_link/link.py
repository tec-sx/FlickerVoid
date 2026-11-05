"""Runs the UE-side snippets in unreal_side/ and returns their results.

Each snippet defines main(params) returning a JSON-serializable dict. The wrapper prints it on one
line after RESULT_MARKER, or the error and traceback when main raises.
"""

import json
import os

from .remote import RemoteError, RemoteSession

RESULT_MARKER = "FVLINK:"

_SNIPPET_DIR = os.path.join(os.path.dirname(__file__), "unreal_side")

_WRAPPER = """
import json as _fv_json
import traceback as _fv_traceback
try:
    _fv_result = {{"ok": True, "value": main(_fv_json.loads({params!r}))}}
except Exception as _fv_error:
    _fv_result = {{"ok": False, "error": str(_fv_error), "traceback": _fv_traceback.format_exc()}}
print({marker!r} + _fv_json.dumps(_fv_result))
"""


class LinkError(RuntimeError):
    pass


def build_script(snippet, params):
    with open(os.path.join(_SNIPPET_DIR, snippet + ".py"), encoding="utf-8") as file:
        body = file.read()
    return body + _WRAPPER.format(params=json.dumps(params), marker=RESULT_MARKER)


def parse_result(command_result):
    """The snippet's return value from a command_result, or LinkError with the editor's error text."""
    outputs = command_result.get("output") or []
    for entry in reversed(outputs):
        text = entry.get("output", "") if isinstance(entry, dict) else str(entry)
        for line in reversed(text.splitlines()):
            if line.startswith(RESULT_MARKER):
                result = json.loads(line[len(RESULT_MARKER):])
                if result.get("ok"):
                    return result.get("value")
                raise LinkError(f"{result.get('error')}\n{result.get('traceback', '')}".strip())

    errors = [entry.get("output", "") for entry in outputs if isinstance(entry, dict) and entry.get("type") == "Error"]
    detail = "\n".join(errors) or str(command_result.get("result", ""))
    raise LinkError(f"The editor returned no result. {detail}".strip())


def call(snippet, params=None, config=None, project_dir=None):
    """Connects to the editor, runs one snippet and returns (value, editor node info)."""
    script = build_script(snippet, params or {})
    try:
        with RemoteSession(config, project_dir) as session:
            return parse_result(session.run(script)), session.remote_node
    except RemoteError as error:
        raise LinkError(str(error)) from error
    except OSError as error:
        raise LinkError(f"Connection failed: {error}") from error
