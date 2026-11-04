"""Character files: the library folder, each file's metadata, and making the open session a library character.

A character's metadata is its MF_Character text block, read from other files without opening them. Its collection is
named after it and marked as an asset in the Base or Hero catalog, so the Asset Browser lists the library's bodies.
"""

import os

import bpy
import bpy.utils.previews

from ... import preferences
from ...core import locker

_previews = None


def library(context=None):
    path = preferences.get(context).library_dir
    return os.path.normpath(bpy.path.abspath(path)) if path else ""


def same_file(a, b):
    if not a or not b:
        return False
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))


def open_character(context=None):
    """The open file's character name, or "" when it isn't one of the library's characters."""
    return locker.character_of(library(context), bpy.data.filepath)


def current_metadata():
    text = bpy.data.texts.get(locker.METADATA)
    return locker.parse_metadata(text.as_string() if text else "")


def write_metadata(metadata):
    text = bpy.data.texts.get(locker.METADATA) or bpy.data.texts.new(locker.METADATA)
    text.clear()
    text.write(locker.format_metadata(metadata))


def read_metadata(path):
    """A character file's metadata; another file is read without opening it."""
    if same_file(path, bpy.data.filepath):
        return current_metadata()
    text = ""
    try:
        with bpy.data.temp_data() as temp:
            with temp.libraries.load(path) as (source, loaded):
                loaded.texts = [locker.METADATA] if locker.METADATA in source.texts else []
            if loaded.texts and loaded.texts[0] is not None:
                text = loaded.texts[0].as_string()
    except (OSError, RuntimeError, ValueError):
        pass
    return locker.parse_metadata(text)


def character_armature(context):
    """The armature picked in Poses, or else the scene's first armature."""
    armature = context.scene.meatbag.armature
    if armature is not None:
        return armature
    return next((obj for obj in context.scene.objects if obj.type == "ARMATURE"), None)


def mark_asset(collection, kind):
    if collection.asset_data is None:
        collection.asset_mark()
    collection.asset_data.catalog_id = locker.CATALOG_IDS[kind]


def _character_collection(scene, previous_name):
    """The collection named after the character, else the scene's only collection, else a new one with the loose objects."""
    collection = bpy.data.collections.get(previous_name) if previous_name else None
    if collection is None and len(scene.collection.children) == 1 and not scene.collection.objects:
        collection = scene.collection.children[0]
    if collection is None:
        collection = bpy.data.collections.new("Character")
        scene.collection.children.link(collection)
        for obj in list(scene.collection.objects):
            collection.objects.link(obj)
            scene.collection.objects.unlink(obj)
    return collection


def adopt(context, name, kind, base, previous_name, clear_links):
    """Makes the open session the character `name`, before it is saved into the library.

    clear_links empties every object's Unreal asset, so a copy can't overwrite the asset of the character it came from."""
    collection = _character_collection(context.scene, previous_name)
    collection.name = name
    if clear_links:
        for obj in bpy.data.objects:
            obj.fv_ue_asset = ""
    armature = character_armature(context)
    write_metadata({"kind": kind, "base": base, "armature": armature.name if armature else ""})
    mark_asset(collection, kind)


def new_empty(context, name, kind):
    """Fills an empty session: the contract's units, the character collection, and the Reference character's armature."""
    scene = context.scene
    units = scene.unit_settings
    units.system = "METRIC"
    units.scale_length = preferences.contract(context).unit_scale
    units.length_unit = "CENTIMETERS"
    collection = bpy.data.collections.new(name)
    scene.collection.children.link(collection)
    folder = library(context)
    reference = locker.reference(folder)
    armature = append_armature(locker.blend_path(folder, reference)) if reference else None
    if armature is not None:
        collection.objects.link(armature)
        scene.meatbag.armature = armature
    write_metadata({"kind": kind, "base": "", "armature": armature.name if armature else ""})
    mark_asset(collection, kind)
    return armature


def append_armature(path):
    """Appends a character file's armature, with its pose library, into the open file; None when it has none."""
    if not os.path.isfile(path):
        return None
    name = read_metadata(path)["armature"]
    with bpy.data.libraries.load(path) as (source, loaded):
        if name in source.objects:
            loaded.objects = [name]
        else:
            loaded.armatures = list(source.armatures)
    obj = loaded.objects[0] if name and loaded.objects else None
    if obj is not None and obj.type == "ARMATURE":
        return obj
    armatures = [data for data in loaded.armatures if data is not None]
    if not armatures:
        return None
    data = max(armatures, key=lambda item: len(item.bones))
    for other in armatures:
        if other != data:
            bpy.data.armatures.remove(other)
    return bpy.data.objects.new(data.name, data)


def thumbnail(path):
    """The icon of a character file's saved preview; 0 while it has none."""
    if _previews is None:
        return 0
    preview = _previews.get(path) or _previews.load(path, path, "BLEND")
    return preview.icon_id


def forget_thumbnails():
    if _previews is not None:
        _previews.clear()


def register_previews():
    global _previews
    _previews = bpy.utils.previews.new()


def unregister_previews():
    global _previews
    if _previews is not None:
        bpy.utils.previews.remove(_previews)
        _previews = None
