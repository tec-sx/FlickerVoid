"""Meat Locker operators: the character list, Add, Copy, Delete, Open and Edit Template, Base or Hero, and the Reference."""

import os

import bpy
from bpy.app.handlers import persistent

from . import characters
from ...core import locker

ASSET_LIBRARY_NAME = "Meat Locker"

KIND_ITEMS = (
    (locker.BASE, "Base", "A base body that other characters start from"),
    (locker.HERO, "Hero", "A story character with its own mesh, detail and correctives, copied from a base"),
)
KIND_LABELS = {identifier: label for identifier, label, _ in KIND_ITEMS}

SOURCE_ITEMS = (
    ("EMPTY", "Empty", "A new file with the Reference character's armature"),
    ("FILE", "This File", "The open file, saved into the library under the new name"),
    ("SELECTED", "Selected Character", "A copy of the character selected in the list"),
)


class MeatLockerCharacter(bpy.types.PropertyGroup):
    path: bpy.props.StringProperty()
    kind: bpy.props.StringProperty()
    base: bpy.props.StringProperty()


def selected(context):
    wm = context.window_manager
    index = wm.meat_locker_index
    return wm.meat_locker[index] if 0 <= index < len(wm.meat_locker) else None


def refresh(context=None, select=None):
    """Lists the library's characters again, keeping the selection unless select names another."""
    context = context or bpy.context
    wm = context.window_manager
    if select is None:
        item = selected(context)
        select = item.name if item else characters.open_character(context)
    library = characters.library(context)
    wm.meat_locker.clear()
    characters.forget_thumbnails()
    for character in locker.characters(library):
        metadata = characters.read_metadata(character.path)
        item = wm.meat_locker.add()
        item.name = character.name
        item.path = character.path
        item.kind = metadata["kind"]
        item.base = metadata["base"]
    names = [item.name for item in wm.meat_locker]
    wm.meat_locker_index = names.index(select) if select in names else 0
    wm.meat_locker_reference = locker.reference(library)


def asset_library(context):
    """Blender's asset library entry for the character library, or None."""
    folder = characters.library(context)
    return next((entry for entry in context.preferences.filepaths.asset_libraries
                 if characters.same_file(bpy.path.abspath(entry.path), folder)), None)


@persistent
def _loaded(*_):
    refresh(select=characters.open_character())


@persistent
def _saving(*_):
    """Keeps a library character's armature name current in its metadata."""
    if bpy.data.texts.get(locker.METADATA) is None:
        return
    armature = characters.character_armature(bpy.context)
    metadata = characters.current_metadata()
    if armature is not None and metadata["armature"] != armature.name:
        metadata["armature"] = armature.name
        characters.write_metadata(metadata)


_HANDLERS = (
    (bpy.app.handlers.load_post, _loaded),
    (bpy.app.handlers.save_pre, _saving),
)


def _refresh_soon():
    try:
        refresh()
    except (AttributeError, RuntimeError):
        pass
    return None


class _LeavesFile:
    """For operators that open another file: offers to save the open one first."""

    save_current: bpy.props.BoolProperty(name="Save Current File", default=True, options={"SKIP_SAVE"},
                                         description="Save the open file before leaving it; off drops its unsaved changes")

    def leaves_file(self, context):
        return True

    def must_ask(self, context):
        return bpy.data.is_dirty and self.leaves_file(context)

    def draw_leaving(self, context, layout):
        if not self.must_ask(context):
            return
        if bpy.data.filepath:
            layout.prop(self, "save_current")
        else:
            layout.label(text="The open file is untitled: its changes will be lost", icon="ERROR")

    def leave(self, context):
        if self.must_ask(context) and bpy.data.filepath and self.save_current:
            bpy.ops.wm.save_mainfile()


class MEATBAG_UL_characters(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        row = layout.row(align=True)
        thumbnail = characters.thumbnail(item.path)
        if thumbnail:
            row.label(text=item.name, icon_value=thumbnail)
        else:
            row.label(text=item.name, icon="USER")
        row.label(text=KIND_LABELS.get(item.kind, ""))
        if item.name == context.window_manager.meat_locker_reference:
            row.label(text="", icon="ARMATURE_DATA")
        if characters.same_file(item.path, bpy.data.filepath):
            row.label(text="", icon="FILE_BLEND")


class MEATBAG_OT_locker_refresh(bpy.types.Operator):
    bl_idname = "meatbag.locker_refresh"
    bl_label = "Refresh"
    bl_description = "List the library's characters again"

    def execute(self, context):
        refresh(context)
        return {"FINISHED"}


class MEATBAG_OT_locker_add(_LeavesFile, bpy.types.Operator):
    bl_idname = "meatbag.locker_add"
    bl_label = "Add Character"
    bl_description = ("Add a character to the library, empty, from the open file or as a copy of the selected character, "
                      "and open it")

    name: bpy.props.StringProperty(name="Name", options={"SKIP_SAVE"})
    kind: bpy.props.EnumProperty(name="Kind", items=KIND_ITEMS)
    source: bpy.props.EnumProperty(name="Start From", items=SOURCE_ITEMS)

    @classmethod
    def poll(cls, context):
        return bool(characters.library(context))

    def _copies_open_file(self, context):
        item = selected(context)
        return self.source == "SELECTED" and item is not None and characters.same_file(item.path, bpy.data.filepath)

    def leaves_file(self, context):
        return self.source != "FILE" and not self._copies_open_file(context)

    def invoke(self, context, event):
        item = selected(context)
        if self.source == "SELECTED" and item is not None:
            self.kind = locker.HERO
            self.name = item.name + " Copy"
        return context.window_manager.invoke_props_dialog(self, width=340)

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "name")
        layout.row().prop(self, "kind", expand=True)
        layout.prop(self, "source")
        if self.source == "SELECTED":
            item = selected(context)
            layout.label(text=f"A copy of {item.name}" if item else "Select a character in the list first", icon="DUPLICATE")
        if self.source == "EMPTY" and not context.window_manager.meat_locker_reference:
            layout.label(text="No Reference character yet: the file starts without an armature", icon="INFO")
        self.draw_leaving(context, layout)

    def execute(self, context):
        library = characters.library(context)
        problem = locker.name_problem(library, self.name)
        if problem:
            self.report({"ERROR"}, problem)
            return {"CANCELLED"}
        item = selected(context)
        if self.source == "SELECTED" and item is None:
            self.report({"ERROR"}, "Select the character to copy in the list")
            return {"CANCELLED"}
        name, kind, path = self.name, self.kind, locker.blend_path(library, self.name)
        source = "FILE" if self._copies_open_file(context) else self.source
        locker.ensure_catalogs(library)
        os.makedirs(os.path.dirname(path), exist_ok=True)

        if source == "FILE":
            previous = characters.open_character(context)
            name_before = previous or (bpy.path.display_name_from_filepath(bpy.data.filepath) if bpy.data.filepath else "")
            characters.adopt(context, name, kind, previous, name_before, clear_links=bool(previous))
        elif source == "SELECTED":
            base, source_path = item.name, item.path
            self.leave(context)
            bpy.ops.wm.open_mainfile(filepath=source_path)
            characters.adopt(bpy.context, name, kind, base, base, clear_links=True)
        else:
            self.leave(context)
            bpy.ops.wm.read_homefile(use_empty=True)
            characters.new_empty(bpy.context, name, kind)
        bpy.ops.wm.save_as_mainfile(filepath=path)
        refresh(bpy.context, select=name)
        return {"FINISHED"}


class MEATBAG_OT_locker_delete(bpy.types.Operator):
    bl_idname = "meatbag.locker_delete"
    bl_label = "Delete Character"
    bl_description = "Move the selected character's folder into the library's _Trash folder"

    @classmethod
    def poll(cls, context):
        item = selected(context)
        return item is not None and not characters.same_file(item.path, bpy.data.filepath)

    def invoke(self, context, event):
        item = selected(context)
        return context.window_manager.invoke_confirm(self, event, title="Delete Character",
                                                     message=f"Move {item.name} into _Trash?", confirm_text="Delete")

    def execute(self, context):
        library = characters.library(context)
        name = selected(context).name
        try:
            target = locker.trash(library, name)
        except OSError as error:
            self.report({"ERROR"}, f"Couldn't move {name}: {error}")
            return {"CANCELLED"}
        if locker.reference(library) == name:
            locker.set_reference(library, "")
        refresh(context, select="")
        self.report({"INFO"}, f"{name} moved to {target}")
        return {"FINISHED"}


class MEATBAG_OT_locker_open(_LeavesFile, bpy.types.Operator):
    bl_idname = "meatbag.locker_open"
    bl_label = "Open"
    bl_description = "Open the selected character's file"

    edit_template: bpy.props.BoolProperty(name="Edit Template", options={"SKIP_SAVE"},
                                          description="Then show the body's half template in Edit Mode")

    @classmethod
    def poll(cls, context):
        return selected(context) is not None

    @classmethod
    def description(cls, context, properties):
        if properties.edit_template:
            return "Open the selected character's file on its half template, in Edit Mode"
        return cls.bl_description

    def leaves_file(self, context):
        item = selected(context)
        return item is not None and not characters.same_file(item.path, bpy.data.filepath)

    def invoke(self, context, event):
        if self.must_ask(context):
            return context.window_manager.invoke_props_dialog(self, width=340)
        return self.execute(context)

    def draw(self, context):
        self.layout.label(text=f"Open {selected(context).name}", icon="FILE_FOLDER")
        self.draw_leaving(context, self.layout)

    def execute(self, context):
        path, edit_template = selected(context).path, self.edit_template
        if self.leaves_file(context):
            self.leave(context)
            bpy.ops.wm.open_mainfile(filepath=path)
        if edit_template and bpy.ops.meatbag.edit_template.poll():
            bpy.ops.meatbag.edit_template()
        return {"FINISHED"}


class MEATBAG_OT_locker_kind(bpy.types.Operator):
    bl_idname = "meatbag.locker_kind"
    bl_label = "Kind"
    bl_description = "Make the open character a Base or a Hero, which also moves it to that asset catalog. Save to keep it"
    bl_options = {"REGISTER", "UNDO"}

    kind: bpy.props.EnumProperty(name="Kind", items=KIND_ITEMS)

    @classmethod
    def poll(cls, context):
        return bool(characters.open_character(context))

    def execute(self, context):
        name = characters.open_character(context)
        metadata = characters.current_metadata()
        metadata["kind"] = self.kind
        characters.write_metadata(metadata)
        collection = bpy.data.collections.get(name)
        if collection is not None:
            characters.mark_asset(collection, self.kind)
        locker.ensure_catalogs(characters.library(context))
        refresh(context)
        return {"FINISHED"}


class MEATBAG_OT_locker_reference(bpy.types.Operator):
    bl_idname = "meatbag.locker_reference"
    bl_label = "Reference"
    bl_description = ("Make the selected character the library's Reference, or stop it being one: the other characters' "
                      "skeletons are checked against it and synced from it")

    @classmethod
    def poll(cls, context):
        return selected(context) is not None

    def execute(self, context):
        library = characters.library(context)
        name = selected(context).name
        new = "" if locker.reference(library) == name else name
        try:
            locker.set_reference(library, new)
        except OSError as error:
            self.report({"ERROR"}, f"Couldn't write {locker.SETTINGS}: {error}")
            return {"CANCELLED"}
        context.window_manager.meat_locker_reference = new
        self.report({"INFO"}, f"{name} is the Reference character" if new else "The library has no Reference character")
        return {"FINISHED"}


class MEATBAG_OT_locker_asset_library(bpy.types.Operator):
    bl_idname = "meatbag.locker_asset_library"
    bl_label = "Add to Asset Libraries"
    bl_description = "Add the character library to Blender's asset libraries, so the Asset Browser lists its characters by Base and Hero"

    @classmethod
    def poll(cls, context):
        return bool(characters.library(context)) and asset_library(context) is None

    def execute(self, context):
        folder = characters.library(context)
        locker.ensure_catalogs(folder)
        context.preferences.filepaths.asset_libraries.new(name=ASSET_LIBRARY_NAME, directory=folder)
        self.report({"INFO"}, f"{folder} added to the asset libraries as {ASSET_LIBRARY_NAME}")
        return {"FINISHED"}


classes = (
    MeatLockerCharacter,
    MEATBAG_UL_characters,
    MEATBAG_OT_locker_refresh,
    MEATBAG_OT_locker_add,
    MEATBAG_OT_locker_delete,
    MEATBAG_OT_locker_open,
    MEATBAG_OT_locker_kind,
    MEATBAG_OT_locker_reference,
    MEATBAG_OT_locker_asset_library,
)


def register_properties():
    wm = bpy.types.WindowManager
    wm.meat_locker = bpy.props.CollectionProperty(type=MeatLockerCharacter)
    wm.meat_locker_index = bpy.props.IntProperty(name="Character")
    wm.meat_locker_reference = bpy.props.StringProperty(name="Reference")
    characters.register_previews()
    for handlers, handler in _HANDLERS:
        handlers.append(handler)
    bpy.app.timers.register(_refresh_soon, first_interval=0.1)


def unregister_properties():
    for handlers, handler in _HANDLERS:
        if handler in handlers:
            handlers.remove(handler)
    if bpy.app.timers.is_registered(_refresh_soon):
        bpy.app.timers.unregister(_refresh_soon)
    characters.unregister_previews()
    wm = bpy.types.WindowManager
    del wm.meat_locker_reference
    del wm.meat_locker_index
    del wm.meat_locker
