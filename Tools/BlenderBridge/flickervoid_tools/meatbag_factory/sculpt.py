"""The managed sculpt target: a hidden copy of the body on the same armature, with one shape key per library pose.

The body stays as it is pushed to Unreal. Sculpting happens on the copy, on the shape key named after the pose shown,
and the solve reads the copy as its target.
"""

TARGET_SUFFIX = "_MF_Target"
TARGET_TAG = "mf_sculpt_target"
BASIS = "Basis"


class SculptError(RuntimeError):
    pass


def effective_target(settings):
    """The picked Target, else the body's managed sculpt target."""
    if settings.target:
        return settings.target
    return settings.mesh.meatbag_target if settings.mesh else None


def ensure_target(mesh):
    target = mesh.meatbag_target
    if target is not None:
        if len(target.data.vertices) != len(mesh.data.vertices):
            raise SculptError(f"{target.name} no longer matches {mesh.name}'s vertices; delete it to make a new one")
        return target

    target = mesh.copy()
    target.data = mesh.data.copy()
    target.name = target.data.name = mesh.name + TARGET_SUFFIX
    from . import muscle_bones
    rig = next((modifier.object for modifier in target.modifiers if modifier.type == "ARMATURE"), None)
    muscle_bones.strip_weights(target, muscle_bones.owned_landmarks(rig))  # the target shows the muscles; muscle bones would push it twice
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


def pose_key(target, name, create=False):
    keys = target.data.shape_keys
    key = keys.key_blocks.get(name) if keys else None
    if key is None and create:
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
