"""Per-pose sculpts: the body's pose blend shapes and the managed sculpt target.

A pose's blend shape is the body's shape key CR_<pose>, sculpted in the pose and sent to Unreal as a morph target the
pose drives. The sculpt target is a hidden copy of the body on the same armature, with one shape key per library pose
holding the full shape wanted in that pose; the solve reads it as its target.
"""

import bpy

SHAPE_PREFIX = "CR_"
TARGET_SUFFIX = "_MF_Target"
TARGET_TAG = "mf_sculpt_target"
BASIS = "Basis"


class SculptError(RuntimeError):
    pass


def is_stale(mesh, target):
    return len(target.data.vertices) != len(mesh.data.vertices)


def stale_message(mesh, target):
    return (f"{target.name} has {len(target.data.vertices)} vertices, {mesh.name} now {len(mesh.data.vertices)}: the body "
            f"changed after the target was made. Press Rebuild Target (its sculpts are lost)")


def remove_target(mesh):
    target = mesh.meatbag_target
    if target is None:
        return
    data = target.data
    mesh.meatbag_target = None
    bpy.data.objects.remove(target)
    if data.users == 0:
        bpy.data.meshes.remove(data)


def ensure_target(mesh):
    target = mesh.meatbag_target
    if target is not None:
        if len(target.data.vertices) != len(mesh.data.vertices):
            raise SculptError(stale_message(mesh, target))
        return target

    target = mesh.copy()
    target.data = mesh.data.copy()
    target.name = target.data.name = mesh.name + TARGET_SUFFIX
    target.shape_key_clear()
    target.shape_key_add(name=BASIS, from_mix=False)
    for modifier in list(target.modifiers):
        if modifier.type != "ARMATURE":
            target.modifiers.remove(modifier)
    target.meatbag_target = None
    target.fv_ue_asset = ""
    target.fv_ue_skeleton = ""
    for collection in mesh.users_collection:
        collection.objects.link(target)
    target[TARGET_TAG] = True
    target.hide_render = True
    target.hide_set(True)
    mesh.meatbag_target = target
    return target


def shape_name(pose):
    return SHAPE_PREFIX + pose


def pose_key(target, name, create=False):
    keys = target.data.shape_keys
    key = keys.key_blocks.get(name) if keys else None
    if key is None and create:
        if not keys:
            target.shape_key_add(name=BASIS, from_mix=False)
        key = target.shape_key_add(name=name, from_mix=False)
        key.value = 0.0
    return key


def rename_pose_key(target, old, new):
    key = pose_key(target, old)
    if key is not None and pose_key(target, new) is None:
        key.name = new


def remove_pose_key(target, name):
    key = pose_key(target, name)
    if key is not None:
        target.shape_key_remove(key)
