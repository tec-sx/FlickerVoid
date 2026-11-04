"""The Meat Locker's character library on disk: one folder per character holding <Name>.blend, plus _Trash."""

import json
import os
import re
import shutil
import time
from dataclasses import dataclass

TRASH = "_Trash"
SETTINGS = "MeatLocker.json"
CATALOGS = "blender_assets.cats.txt"
METADATA = "MF_Character"

BASE = "BASE"
HERO = "HERO"
KINDS = (BASE, HERO)

# Fixed ids, so every character file in every library agrees on the two catalogs.
CATALOG_IDS = {BASE: "5b1c4d0e-6f27-4a8e-9d3b-2c7a1e5f8b01", HERO: "9e2f7a43-1c8d-4b6e-a5f0-3d9b8c2e7a12"}
CATALOG_PATHS = {BASE: "Meat Locker/Base", HERO: "Meat Locker/Hero"}
_CATALOG_HEADER = (
    "# This is an Asset Catalog Definition file for Blender.\n"
    "#\n"
    "# Empty lines and lines starting with `#` will be ignored.\n"
    "# The first non-ignored line should be the version indicator.\n"
    '# Other lines are of the format "UUID:catalog/path/for/assets:simple catalog name"\n'
    "\n"
    "VERSION 1\n"
    "\n"
)

_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9 _-]*$")


@dataclass
class Character:
    name: str
    path: str


def blend_path(library, name):
    return os.path.join(library, name, name + ".blend")


def characters(library):
    """The library's characters by name: each folder not starting with _ or . that holds <folder>.blend."""
    if not library or not os.path.isdir(library):
        return []
    found = []
    for entry in sorted(os.listdir(library), key=str.lower):
        if entry.startswith(("_", ".")):
            continue
        path = blend_path(library, entry)
        if os.path.isfile(path):
            found.append(Character(entry, path))
    return found


def character_of(library, filepath):
    """The character's name when filepath is a character's .blend in the library, else ""."""
    if not library or not filepath:
        return ""
    folder, file = os.path.split(os.path.normpath(filepath))
    name = os.path.basename(folder)
    if os.path.normcase(os.path.dirname(folder)) != os.path.normcase(os.path.normpath(library)):
        return ""
    if name.startswith(("_", ".")) or file != name + ".blend":
        return ""
    return name


def name_problem(library, name):
    """Why a new character can't take this name, or "" when it can."""
    if not _NAME.match(name or ""):
        return "Use letters, digits, spaces, _ and -, starting with a letter or digit"
    if os.path.exists(os.path.join(library, name)):
        return f"{name} already exists in the library"
    return ""


def trash(library, name):
    """Moves a character's folder into _Trash, dated when the name is taken there; returns the new folder."""
    target_dir = os.path.join(library, TRASH)
    os.makedirs(target_dir, exist_ok=True)
    target = os.path.join(target_dir, name)
    if os.path.exists(target):
        target += time.strftime(" %Y-%m-%d %H-%M-%S")
    shutil.move(os.path.join(library, name), target)
    return target


def read_settings(library):
    try:
        with open(os.path.join(library, SETTINGS), encoding="utf-8") as file:
            settings = json.load(file)
    except (OSError, ValueError):
        return {}
    return settings if isinstance(settings, dict) else {}


def write_settings(library, settings):
    with open(os.path.join(library, SETTINGS), "w", encoding="utf-8") as file:
        json.dump(settings, file, indent=2, sort_keys=True)
        file.write("\n")


def reference(library):
    """The name of the library's Reference character, whose skeleton the others are checked against."""
    return read_settings(library).get("reference", "") if library else ""


def set_reference(library, name):
    settings = read_settings(library)
    settings["reference"] = name
    write_settings(library, settings)


def parse_metadata(text):
    """A character's metadata from its MF_Character text block: kind, the base it came from, and its armature."""
    try:
        data = json.loads(text or "{}")
    except ValueError:
        data = {}
    if not isinstance(data, dict):
        data = {}
    kind = data.get("kind", BASE)
    return {
        "kind": kind if kind in KINDS else BASE,
        "base": str(data.get("base", "")),
        "armature": str(data.get("armature", "")),
    }


def format_metadata(metadata):
    return json.dumps(parse_metadata(json.dumps(metadata)), indent=2, sort_keys=True) + "\n"


def ensure_catalogs(library):
    """Adds the Base and Hero catalogs to the library's catalog file, keeping the ones already there."""
    path = os.path.join(library, CATALOGS)
    try:
        with open(path, encoding="utf-8") as file:
            text = file.read()
    except OSError:
        text = _CATALOG_HEADER
    lines = [f"{CATALOG_IDS[kind]}:{CATALOG_PATHS[kind]}:{CATALOG_PATHS[kind].replace('/', '-')}"
             for kind in KINDS if CATALOG_IDS[kind] not in text]
    if not lines:
        return False
    if not text.endswith("\n"):
        text += "\n"
    with open(path, "w", encoding="utf-8") as file:
        file.write(text + "\n".join(lines) + "\n")
    return True
