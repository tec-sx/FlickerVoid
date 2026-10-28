"""Writes a solve back to the scene: helper bone keys per pose, helper weights, residual error and residual shape keys."""

import numpy as np
from mathutils import Matrix

from . import solver

RESIDUAL_ATTRIBUTE = "mf_residual"
RESIDUAL_PREFIX = "CR_"
WEIGHT_CHANGE = 1e-6


def _matrix(array):
    return Matrix([list(row) for row in array])


def _depth(bone):
    return len(bone.parent_recursive)


def write_helper_keys(context, armature, capture, result):
    """Keys every helper bone at each pose's frame so its posed matrix matches the solve."""
    scene = context.scene
    index = {name: i for i, name in enumerate(capture.bones)}
    helpers = sorted((armature.data.bones[name] for name, i in index.items() if capture.problem.helpers[i]), key=_depth)
    frame = scene.frame_current
    try:
        for pose_index, (_, pose_frame) in enumerate(capture.poses):
            scene.frame_set(pose_frame)
            for bone in helpers:
                b = index[bone.name]
                skin = np.eye(4)
                skin[:3, :] = result.skin[pose_index, b]
                pose_bone = armature.pose.bones[bone.name]
                pose_bone.matrix = _matrix(skin @ capture.bind[b])
                context.view_layer.update()
                _key(pose_bone, pose_frame)
    finally:
        scene.frame_set(frame)
    return len(helpers)


def _key(pose_bone, frame):
    group = pose_bone.name
    pose_bone.keyframe_insert("location", frame=frame, group=group)
    rotation = {"QUATERNION": "rotation_quaternion", "AXIS_ANGLE": "rotation_axis_angle"}.get(pose_bone.rotation_mode, "rotation_euler")
    pose_bone.keyframe_insert(rotation, frame=frame, group=group)
    pose_bone.keyframe_insert("scale", frame=frame, group=group)


def write_weights(mesh, capture, weights):
    """Writes the vertex weights that changed; returns how many vertices changed."""
    changed = np.flatnonzero(np.abs(weights - capture.problem.weights).max(axis=1) > WEIGHT_CHANGE)
    groups = {}
    for bone in capture.bones:
        groups[bone] = mesh.vertex_groups.get(bone)
    for vertex in changed:
        for b in np.flatnonzero((weights[vertex] > 0) | (capture.problem.weights[vertex] > 0)):
            name = capture.bones[b]
            group = groups[name] or mesh.vertex_groups.new(name=name)
            groups[name] = group
            if weights[vertex, b] > 0:
                group.add([int(vertex)], float(weights[vertex, b]), "REPLACE")
            else:
                group.remove([int(vertex)])
    return len(changed)


def write_residual(mesh, result):
    """Stores each vertex's largest distance to the target over all poses as a point attribute."""
    values = result.error.max(axis=0).astype(np.float32)
    attribute = mesh.data.attributes.get(RESIDUAL_ATTRIBUTE)
    if attribute and (attribute.domain != "POINT" or attribute.data_type != "FLOAT"):
        mesh.data.attributes.remove(attribute)
        attribute = None
    attribute = attribute or mesh.data.attributes.new(RESIDUAL_ATTRIBUTE, "FLOAT", "POINT")
    attribute.data.foreach_set("value", values)
    mesh.data.update()


def write_residual_shape_keys(mesh, capture, result, threshold):
    """One shape key per pose (CR_<pose>) holding what the bones couldn't reach, moved back to rest space."""
    problem = capture.problem
    residual = problem.targets - solver.skin_points(problem.rest, result.weights, result.skin)
    to_mesh = np.linalg.inv(capture.to_armature[:3, :3])
    if not mesh.data.shape_keys:
        mesh.shape_key_add(name="Basis", from_mix=False)
    basis = np.empty(len(mesh.data.vertices) * 3)
    mesh.data.vertices.foreach_get("co", basis)
    basis = basis.reshape(-1, 3)

    written = []
    for pose_index, (pose, _) in enumerate(capture.poses):
        if result.error[pose_index].max() <= threshold:
            continue
        blended = np.einsum("nb,bij->nij", result.weights, result.skin[pose_index][:, :, :3])
        delta = np.linalg.solve(blended, residual[pose_index][:, :, None])[:, :, 0]
        name = RESIDUAL_PREFIX + pose
        key = mesh.data.shape_keys.key_blocks.get(name) or mesh.shape_key_add(name=name, from_mix=False)
        key.data.foreach_set("co", (basis + delta @ to_mesh.T).reshape(-1))
        key.value = 0.0
        written.append(name)
    mesh.data.update()
    return written
