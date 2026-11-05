"""Reads Unreal config ini files, applying the array operators (+, -, ., !) the way the engine does."""

import re

_STRUCT_FIELD = re.compile(r'(\w+)\s*=\s*(?:"((?:[^"\\]|\\.)*)"|([^,)]*))')


def read_ini(path):
    """Returns {section: {key: [values]}}. A key that is present but cleared maps to an empty list."""
    with open(path, encoding="utf-8-sig") as file:
        return parse_ini(file.read())


def parse_ini(text):
    sections = {}
    current = None
    for raw in text.splitlines():
        line = raw.strip()
        if not line or line[0] in ";#":
            continue
        if line.startswith("[") and line.endswith("]"):
            current = sections.setdefault(line[1:-1], {})
            continue
        if current is None:
            continue

        op = line[0] if line[0] in "+-.!" else ""
        key, _, value = (line[1:] if op else line).partition("=")
        key, value = key.strip(), value.strip()
        values = current.setdefault(key, [])
        if op == "!":
            values.clear()
        elif op == "+":
            if value not in values:
                values.append(value)
        elif op == ".":
            values.append(value)
        elif op == "-":
            if value in values:
                values.remove(value)
        else:
            values[:] = [value]
    return sections


def parse_struct(value):
    """Parses a struct value such as (Left="_l",Right="_r") into a dict of strings."""
    inner = value.strip()
    if inner.startswith("(") and inner.endswith(")"):
        inner = inner[1:-1]
    fields = {}
    for match in _STRUCT_FIELD.finditer(inner):
        quoted, bare = match.group(2), match.group(3)
        fields[match.group(1)] = quoted.replace('\\"', '"') if quoted is not None else bare.strip()
    return fields
