"""Reads a corrective problem from the scene: rest mesh, weights, posed bones and target meshes per pose."""

from contextlib import contextmanager
from dataclasses import dataclass

import numpy as np
from mathutils import Matrix

from . import solver


@dataclass
class Capture:
    problem: solver.Problem
    bones: list          # deform bone names, in Problem order
    poses: list          # (pose name, frame)
    bind: np.ndarray     # (B, 4, 4) bone rest matrices in armature space
    posed: np.ndarray    # (F, B, 4, 4) posed bone matrices with helpers at rest
    to_armature: np.ndarray  # (4, 4) mesh space to armature space


class CaptureError(RuntimeError):
    pass


def to_numpy(matrix):
    return np.array([list(row) for row in matrix], dtype=float)


def pose_markers(scene):
    """(name, frame) of the scene's timeline markers, in frame order: one marker per example pose."""
    markers = sorted(scene.timeline_markers, key=lambda marker: marker.frame)
    names = [marker.name for marker in markers]
    duplicates = sorted({name for name in names if names.count(name) > 1})
    if duplicates:
        raise CaptureError(f"Pose markers must have unique names: {', '.join(duplicates)}")
    return [(marker.name, marker.frame) for marker in markers]


def helper_bone_names(armature, collection_name):
    """Deform bones in the named bone collection, else the selected deform bones."""
    data = armature.data
    collections = getattr(data, "collections_all", data.collections)
    collection = collections.get(collection_name) if collection_name else None
    if collection:
        bones = list(collection.bones)
    else:
        bones = [bone for bone in data.bones if _selected(armature, bone)]
    return [bone.name for bone in bones if bone.use_deform]


def _selected(armature, bone):
    # Blender 5 keeps pose selection on the pose bone, 4.x on the bone.
    pose_bone = armature.pose.bones[bone.name]
    return pose_bone.select if hasattr(pose_bone, "select") else bone.select


def action_fcurves(armature):
    animation = armature.animation_data
    action = animation.action if animation else None
    if not action:
        return []
    try:
        from bpy_extras import anim_utils
        channelbag = anim_utils.action_get_channelbag_for_slot(action, animation.action_slot)
        return list(channelbag.fcurves) if channelbag else []
    except (ImportError, AttributeError):
        return list(action.fcurves)


def bone_fcurves(armature, bone_names):
    prefixes = tuple(f'pose.bones["{name}"]' for name in bone_names)
    return [fcurve for fcurve in action_fcurves(armature) if fcurve.data_path.startswith(prefixes)]


@contextmanager
def helpers_at_rest(armature, helper_names):
    """Mutes the helpers' animation and resets their pose, so captures show main bones and plain skinning only."""
    fcurves = bone_fcurves(armature, helper_names)
    muted = [(fcurve, fcurve.mute) for fcurve in fcurves]
    bases = {name: armature.pose.bones[name].matrix_basis.copy() for name in helper_names}
    for fcurve in fcurves:
        fcurve.mute = True
    try:
        yield
    finally:
        for fcurve, mute in muted:
            fcurve.mute = mute
        for name, basis in bases.items():
            armature.pose.bones[name].matrix_basis = basis


def _reset_helpers(armature, helper_names, view_layer):
    for name in helper_names:
        armature.pose.bones[name].matrix_basis = Matrix.Identity(4)
    view_layer.update()


def capture(context, armature, mesh, target, helper_names):
    scene = context.scene
    poses = pose_markers(scene)
    if not poses:
        raise CaptureError("Add a timeline marker for each example pose, named after the pose (thigh_l_raise)")
    if not helper_names:
        raise CaptureError("No helper bones: put them in the helper bone collection, or select them")
    for modifier in mesh.modifiers:
        if modifier.type == "ARMATURE" and modifier.use_deform_preserve_volume:
            raise CaptureError(f"{mesh.name}: turn off Preserve Volume on the Armature modifier; Unreal skins linearly")

    bones = [bone.name for bone in armature.data.bones if bone.use_deform]
    index = {name: i for i, name in enumerate(bones)}
    helpers = np.zeros(len(bones), dtype=bool)
    helpers[[index[name] for name in helper_names]] = True

    to_armature = to_numpy(armature.matrix_world.inverted() @ mesh.matrix_world)
    count = len(mesh.data.vertices)
    co = np.empty(count * 3)
    mesh.data.vertices.foreach_get("co", co)
    rest = _transform(co.reshape(-1, 3), to_armature)
    weights = _weights(mesh, index, count)

    bind = np.stack([to_numpy(armature.data.bones[name].matrix_local) for name in bones])
    bind_inverse = np.linalg.inv(bind)
    target_to_armature = to_numpy(armature.matrix_world.inverted() @ target.matrix_world)

    frame = scene.frame_current
    posed_all, skin_all, targets = [], [], []
    try:
        with helpers_at_rest(armature, helper_names):
            for name, pose_frame in poses:
                scene.frame_set(pose_frame)
                _reset_helpers(armature, helper_names, context.view_layer)
                posed = np.stack([to_numpy(armature.pose.bones[bone].matrix) for bone in bones])
                posed_all.append(posed)
                skin_all.append((posed @ bind_inverse)[:, :3, :])
                targets.append(_evaluated_points(context, target, count, target_to_armature, name))
    finally:
        scene.frame_set(frame)

    problem = solver.Problem(rest, weights, np.stack(skin_all), np.stack(targets), helpers)
    return Capture(problem, bones, poses, bind, np.stack(posed_all), to_armature)


def _transform(points, matrix):
    return points @ matrix[:3, :3].T + matrix[:3, 3]


def _weights(mesh, index, count):
    weights = np.zeros((count, len(index)))
    groups = {group.index: index[group.name] for group in mesh.vertex_groups if group.name in index}
    for vertex in mesh.data.vertices:
        for element in vertex.groups:
            bone = groups.get(element.group)
            if bone is not None:
                weights[vertex.index, bone] = element.weight
    totals = weights.sum(axis=1, keepdims=True)
    return np.divide(weights, totals, out=np.zeros_like(weights), where=totals > 0)


def _evaluated_points(context, target, count, matrix, pose_name):
    evaluated = target.evaluated_get(context.evaluated_depsgraph_get())
    data = evaluated.to_mesh()
    try:
        if len(data.vertices) != count:
            raise CaptureError(f"{pose_name}: the target has {len(data.vertices)} vertices, the mesh {count}; the target's modifiers must not change topology")
        co = np.empty(count * 3)
        data.vertices.foreach_get("co", co)
    finally:
        evaluated.to_mesh_clear()
    return _transform(co.reshape(-1, 3), matrix)
