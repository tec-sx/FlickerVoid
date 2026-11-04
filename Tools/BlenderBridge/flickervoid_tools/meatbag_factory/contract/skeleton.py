"""Skeleton checks and sync against the Meat Locker's Reference character, read from its file without opening it.

Every character has its own armature and rest pose with the same bone names and hierarchy. Check Skeleton compares the
deform bones' parents and rolls with the reference; Sync Skeleton adds the reference bones a character lacks, placed
relative to their parent and scaled by the parent's length.
"""

from dataclasses import dataclass

import bpy
from mathutils import Matrix

from ..locker import characters
from ...core import locker
from ...core import skeleton as rules


_MODES = {"POSE": "POSE", "EDIT_ARMATURE": "EDIT"}


class SkeletonError(RuntimeError):
    pass


@dataclass
class Bone:
    parent: object
    matrix: Matrix
    length: float
    connected: bool
    collections: tuple

    @property
    def roll(self):
        return bpy.types.Bone.AxisRollFromMatrix(self.matrix.to_3x3())[1]


def read(armature_data):
    """The deform bones as Unreal gets them, each parented to its nearest deform ancestor."""
    bones = {}
    for bone in armature_data.bones:
        if not bone.use_deform:
            continue
        parent = bone.parent
        while parent is not None and not parent.use_deform:
            parent = parent.parent
        bones[bone.name] = Bone(parent.name if parent else None, bone.matrix_local.copy(), bone.length,
                                bone.use_connect and parent == bone.parent, tuple(item.name for item in bone.collections))
    return bones


def infos(bones):
    return {name: rules.BoneInfo(bone.parent, bone.roll) for name, bone in bones.items()}


def reference_bones(context):
    """(Reference name, its deform bones). Raises SkeletonError when there is no reference to compare with."""
    folder = characters.library(context)
    name = locker.reference(folder)
    if not name:
        raise SkeletonError("The library has no Reference character: pick one in the Meat Locker")
    path = locker.blend_path(folder, name)
    if characters.same_file(path, bpy.data.filepath):
        raise SkeletonError(f"This file is {name}, the Reference character")
    armature_name = characters.read_metadata(path)["armature"]
    try:
        with bpy.data.temp_data() as temp:
            with temp.libraries.load(path) as (source, loaded):
                if armature_name in source.objects:
                    loaded.objects = [armature_name]
                else:
                    loaded.armatures = list(source.armatures)
            data = None
            if loaded.objects and loaded.objects[0] is not None and loaded.objects[0].type == "ARMATURE":
                data = loaded.objects[0].data
            elif loaded.armatures:
                data = max((item for item in loaded.armatures if item is not None), key=lambda item: len(item.bones), default=None)
            if data is None:
                raise SkeletonError(f"{name} has no armature")
            return name, read(data)
    except OSError as error:
        raise SkeletonError(f"Couldn't read {path}: {error}") from error


def check(context, armature):
    """Issues for the armature's deform bones against the Reference character's."""
    _, reference = reference_bones(context)
    return rules.compare(armature.name, infos(reference), infos(read(armature.data)))


def _placement(reference, name, edit_bones):
    """Armature-space matrix and length for a reference bone, relative to its parent as the armature has it."""
    bone = reference[name]
    if bone.parent is None:
        return bone.matrix.copy(), bone.length
    parent_reference = reference[bone.parent]
    parent = edit_bones[bone.parent]
    scale = parent.length / parent_reference.length if parent_reference.length > 1e-9 else 1.0
    relative = parent_reference.matrix.inverted() @ bone.matrix
    relative.translation *= scale
    return parent.matrix @ relative, bone.length * scale


def sync(context, armature):
    """Adds the reference bones the armature lacks; returns their names."""
    _, reference = reference_bones(context)
    order = rules.sync_order(infos(reference), infos(read(armature.data)))
    if not order:
        return []
    view_layer = context.view_layer
    active, mode, hidden = view_layer.objects.active, context.mode, armature.hide_get()
    if mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    armature.hide_set(False)
    view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="EDIT")
    try:
        edit_bones = armature.data.edit_bones
        for name in order:
            matrix, length = _placement(reference, name, edit_bones)
            bone = edit_bones.new(name)
            bone.head = (0.0, 0.0, 0.0)
            bone.tail = (0.0, max(length, 1e-4), 0.0)
            bone.matrix = matrix
            source = reference[name]
            if source.parent is not None:
                bone.parent = edit_bones[source.parent]
                bone.use_connect = source.connected
            bone.use_deform = True
            for collection_name in source.collections:
                collection = armature.data.collections_all.get(collection_name) or armature.data.collections.new(collection_name)
                collection.assign(bone)
    finally:
        bpy.ops.object.mode_set(mode="OBJECT")
        if active == armature and mode in _MODES:
            bpy.ops.object.mode_set(mode=_MODES[mode])
        armature.hide_set(hidden)
        view_layer.objects.active = active
    return order
