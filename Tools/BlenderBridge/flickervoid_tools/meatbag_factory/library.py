"""The pose library: an armature's example poses kept in one action that is never assigned, so the timeline stays free.

Pose i is keyed at frame i on every bone, as location, quaternion rotation and scale relative to the rest pose
(the bone's matrix_basis). Pose names and preview weights live on the action, so the action is the whole library.
Commands read the library into an array, change it and write it back whole, which keeps frames in list order.
"""

import random
from contextlib import contextmanager

import numpy as np

import bpy
from mathutils import Matrix, Quaternion, Vector

CHANNELS = (("location", 3), ("rotation_quaternion", 4), ("scale", 3))
WIDTH = 10
REST = np.array([0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0])

_cache = {}


def library_of(armature, create=False):
    action = armature.meatbag_library
    if action is None and create:
        action = bpy.data.actions.new(f"MF_{armature.name}_Poses")
        action.use_fake_user = True
        armature.meatbag_library = action
    return action


def pose_names(action):
    return [pose.name for pose in action.meatbag_poses] if action else []


def bone_names(armature):
    return [pose_bone.name for pose_bone in armature.pose.bones]


# Storage

def _channelbag(action, create):
    if not hasattr(action, "layers"):
        return action  # Blender before 4.4: legacy actions hold the F-curves directly
    from bpy_extras import anim_utils
    slot = action.slots[0] if action.slots else (action.slots.new(id_type="OBJECT", name="Armature") if create else None)
    if slot is None:
        return None
    if create:
        return anim_utils.action_ensure_channelbag_for_slot(action, slot)
    return anim_utils.action_get_channelbag_for_slot(action, slot)


def _revision(action):
    return action.get("mf_revision", 0)


def read(action, bones):
    """(P, B, 10) pose values for the given bones; bones the library has no keys for are at rest."""
    count = len(action.meatbag_poses) if action else 0
    key = (action.name_full if action else None, _revision(action) if action else 0, tuple(bones))
    cached = _cache.get(key)
    if cached is not None and len(cached) == count:
        return cached.copy()

    values = np.tile(REST, (count, len(bones), 1))
    channelbag = _channelbag(action, False) if action else None
    if channelbag is not None and count:
        index = {name: i for i, name in enumerate(bones)}
        frames = np.arange(count, dtype=float)
        for fcurve in channelbag.fcurves:
            bone, offset = _channel(fcurve)
            if bone not in index or offset is None:
                continue
            values[:, index[bone], offset + fcurve.array_index] = [fcurve.evaluate(frame) for frame in frames]
    _cache.clear()
    _cache[key] = values.copy()
    return values


def _channel(fcurve):
    path = fcurve.data_path
    if not path.startswith('pose.bones["'):
        return None, None
    bone, _, prop = path[len('pose.bones["'):].partition('"].')
    offset = 0
    for name, size in CHANNELS:
        if prop == name:
            return bone, offset
        offset += size
    return bone, None


def write(action, names, bones, values):
    """Replaces the whole library: pose i keyed at frame i, names kept in order, preview weights kept by name."""
    weights = {pose.name: pose.weight for pose in action.meatbag_poses}
    channelbag = _channelbag(action, True)
    for fcurve in list(channelbag.fcurves):
        channelbag.fcurves.remove(fcurve)

    count = len(names)
    frames = np.arange(count, dtype=float)
    if count:
        for b, bone in enumerate(bones):
            offset = 0
            for prop, size in CHANNELS:
                for axis in range(size):
                    fcurve = _new_fcurve(channelbag, f'pose.bones["{bone}"].{prop}', axis, bone)
                    fcurve.keyframe_points.add(count)
                    co = np.empty(count * 2)
                    co[0::2] = frames
                    co[1::2] = values[:, b, offset + axis]
                    fcurve.keyframe_points.foreach_set("co", co)
                    fcurve.keyframe_points.foreach_set("interpolation", [_LINEAR] * count)
                    fcurve.update()
                offset += size

    action.meatbag_poses.clear()
    for name in names:
        pose = action.meatbag_poses.add()
        pose.name = name
        pose["weight"] = weights.get(name, 0.0)  # item access skips the update callback
    # A fresh token rather than a counter, so undo and redo never meet a cached read of another state.
    action["mf_revision"] = random.getrandbits(31)
    _cache.clear()


_LINEAR = bpy.types.Keyframe.bl_rna.properties["interpolation"].enum_items["LINEAR"].value


def _new_fcurve(channelbag, path, index, group):
    if hasattr(channelbag, "fcurves") and hasattr(channelbag.fcurves, "new"):
        try:
            return channelbag.fcurves.new(path, index=index, group_name=group)
        except TypeError:
            return channelbag.fcurves.new(path, index=index, action_group=group)
    raise RuntimeError("This Blender version can't create F-curves")


# Pose values

def current(armature, bones):
    """(B, 10) values of the armature's current pose (matrix_basis), quaternions on the identity's side."""
    values = np.empty((len(bones), WIDTH))
    for b, name in enumerate(bones):
        location, rotation, scale = armature.pose.bones[name].matrix_basis.decompose()
        if rotation.w < 0.0:
            rotation.negate()
        values[b] = [*location, *rotation, *scale]
    return values


def set_pose(armature, bones, values):
    for b, name in enumerate(bones):
        row = values[b]
        rotation = Quaternion(row[3:7])
        rotation.normalize()
        armature.pose.bones[name].matrix_basis = Matrix.LocRotScale(Vector(row[0:3]), rotation, Vector(row[7:10]))


def unique_name(action, base):
    names = set(pose_names(action))
    name = base
    suffix = 1
    while name in names:
        name = f"{base}_{suffix}"
        suffix += 1
    return name


@contextmanager
def preserved_pose(armature):
    """Restores every bone's matrix_basis afterwards."""
    bases = {pose_bone.name: pose_bone.matrix_basis.copy() for pose_bone in armature.pose.bones}
    try:
        yield
    finally:
        for name, basis in bases.items():
            armature.pose.bones[name].matrix_basis = basis


# Preview blending

BLEND = "BLEND"
ADDITIVE = "ADDITIVE"


def _slerp_from_identity(quaternions, weight):
    """slerp(identity, q, weight) for each row of (B, 4) quaternions."""
    w = np.clip(quaternions[:, 0], -1.0, 1.0)
    angle = np.arccos(w)
    sin = np.sin(angle)
    small = sin < 1e-8
    scale_identity = np.where(small, 1.0 - weight, np.sin((1.0 - weight) * angle) / np.where(small, 1.0, sin))
    scale_q = np.where(small, weight, np.sin(weight * angle) / np.where(small, 1.0, sin))
    result = quaternions * scale_q[:, None]
    result[:, 0] += scale_identity
    return result / np.linalg.norm(result, axis=1, keepdims=True)


def _multiply(a, b):
    aw, ax, ay, az = a.T
    bw, bx, by, bz = b.T
    return np.stack([aw * bw - ax * bx - ay * by - az * bz,
                     aw * bx + ax * bw + ay * bz - az * by,
                     aw * by - ax * bz + ay * bw + az * bx,
                     aw * bz + ax * by - ay * bx + az * bw], axis=1)


def mix(values, weights, mode):
    """(B, 10) pose from library values (P, B, 10) and per-pose weights.

    BLEND is a weighted average with the rest pose taking whatever weight is left below 1, so poses A and B at
    1 - t and t interpolate between them. ADDITIVE adds each pose's change from rest on top of the others.
    """
    weights = np.asarray(weights, dtype=float)
    bones = values.shape[1]
    result = np.tile(REST, (bones, 1))
    active = np.flatnonzero(np.abs(weights) > 1e-6)
    if not len(active):
        return result

    if mode == ADDITIVE:
        for p in active:
            w = weights[p]
            result[:, 0:3] += w * values[p, :, 0:3]
            result[:, 3:7] = _multiply(result[:, 3:7], _slerp_from_identity(values[p, :, 3:7], w))
            result[:, 7:10] *= 1.0 + w * (values[p, :, 7:10] - 1.0)
        return result

    weights = np.clip(weights, 0.0, None)
    total = weights.sum()
    rest_weight = max(0.0, 1.0 - total)
    norm = max(total, 1.0)
    result[:, 0:3] = np.einsum("p,pbi->bi", weights, values[:, :, 0:3]) / norm
    quaternions = values[:, :, 3:7] * np.sign(values[:, :, 3:4] + 1e-12)
    blended = np.einsum("p,pbi->bi", weights, quaternions) / norm
    blended[:, 0] += rest_weight / norm
    result[:, 3:7] = blended / np.linalg.norm(blended, axis=1, keepdims=True)
    result[:, 7:10] = (np.einsum("p,pbi->bi", weights, values[:, :, 7:10]) + rest_weight) / norm
    return result
