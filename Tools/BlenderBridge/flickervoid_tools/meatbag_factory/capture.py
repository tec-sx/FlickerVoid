"""Reads a corrective problem from the scene: rest mesh, weights, and per library pose the posed bones and the target."""

from contextlib import contextmanager
from dataclasses import dataclass

import numpy as np
from mathutils import Matrix

from . import library, solver


@dataclass
class Capture:
    problem: solver.Problem
    bones: list          # deform bone names, in Problem order
    poses: list          # names of the captured poses, in library order
    indices: list        # their library indices
    bind: np.ndarray     # (B, 4, 4) bone rest matrices in armature space
    posed: np.ndarray    # (F, B, 4, 4) posed bone matrices with helpers at rest
    to_armature: np.ndarray  # (4, 4) mesh space to armature space


class CaptureError(RuntimeError):
    pass


def to_numpy(matrix):
    return np.array([list(row) for row in matrix], dtype=float)


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


def check_names(names):
    if not names:
        raise CaptureError("The pose library is empty: add a pose for each example")
    duplicates = sorted({name for name in names if names.count(name) > 1})
    if duplicates:
        raise CaptureError(f"Pose names must be unique: {', '.join(duplicates)}")


@contextmanager
def target_shape_keys(target, poses):
    """Target shape keys named after poses are switched on only for their own pose; restored afterwards."""
    keys = target.data.shape_keys.key_blocks if target.data.shape_keys else []
    pose_keys = {key.name: key for key in keys if key.name in poses}
    values = {name: key.value for name, key in pose_keys.items()}
    show_only = target.show_only_shape_key
    target.show_only_shape_key = False
    try:
        yield pose_keys
    finally:
        target.show_only_shape_key = show_only
        for name, value in values.items():
            pose_keys[name].value = value


def show_pose(context, armature, bones, values, helper_names=()):
    """Sets the armature to a library pose, helpers at rest when given, and evaluates the scene."""
    library.set_pose(armature, bones, values)
    for name in helper_names:
        armature.pose.bones[name].matrix_basis = Matrix.Identity(4)
    context.view_layer.update()


def capture(context, armature, mesh, target, helper_names, pose_indices=None):
    """The problem for the given library poses (all when None); helpers not named stay as the library poses them."""
    action = library.library_of(armature)
    names = library.pose_names(action)
    check_names(names)
    indices = list(range(len(names))) if pose_indices is None else list(pose_indices)
    if not indices:
        raise CaptureError("No poses to solve: every pose is locked")
    poses = [names[i] for i in indices]
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

    all_bones = library.bone_names(armature)
    values = library.read(action, all_bones)
    posed_all, skin_all, targets = [], [], []
    with library.preserved_pose(armature), target_shape_keys(target, names) as pose_keys:
        for p, name in zip(indices, poses):
            for key_name, key in pose_keys.items():
                key.value = 1.0 if key_name == name else 0.0
            show_pose(context, armature, all_bones, values[p], helper_names)
            posed = np.stack([to_numpy(armature.pose.bones[bone].matrix) for bone in bones])
            posed_all.append(posed)
            skin_all.append((posed @ bind_inverse)[:, :3, :])
            targets.append(_evaluated_points(context, target, count, target_to_armature, name))
    context.view_layer.update()

    problem = solver.Problem(rest, weights, np.stack(skin_all), np.stack(targets), helpers)
    return Capture(problem, bones, poses, indices, bind, np.stack(posed_all), to_armature)


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
