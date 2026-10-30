"""Bakes the library poses into an armature-only FBX animation, one frame per pose, for a Pose Asset in Unreal."""

from contextlib import contextmanager

import bpy

from . import library
from ..push import export

POSE_ACTION = "MF_PoseAssetSource"


def sample_poses(context, armature):
    """Armature-space matrices of every bone in each library pose, after constraints and drivers."""
    action = library.library_of(armature)
    bones = library.bone_names(armature)
    values = library.read(action, bones)
    samples = []
    with library.preserved_pose(armature):
        for pose_values in values:
            library.set_pose(armature, bones, pose_values)
            context.view_layer.update()
            samples.append({pose_bone.name: pose_bone.matrix.copy() for pose_bone in armature.pose.bones})
    context.view_layer.update()
    return samples


def _basis(bone, samples):
    parent = bone.parent
    if parent:
        return bone.convert_local_to_pose(samples[bone.name], bone.matrix_local, parent_matrix=samples[parent.name],
                                          parent_matrix_local=parent.matrix_local, invert=True)
    return bone.convert_local_to_pose(samples[bone.name], bone.matrix_local, invert=True)


@contextmanager
def baked_copy(context, armature, samples):
    """A temporary copy of the armature without constraints or drivers, keyed with sample i at frame i."""
    copy = armature.copy()
    copy.animation_data_clear()
    for pose_bone in copy.pose.bones:
        for constraint in list(pose_bone.constraints):
            pose_bone.constraints.remove(constraint)
    context.scene.collection.objects.link(copy)
    action = bpy.data.actions.new(POSE_ACTION)
    try:
        copy.animation_data_create().action = action
        for frame, sample in enumerate(samples):
            for bone in copy.data.bones:
                pose_bone = copy.pose.bones[bone.name]
                pose_bone.matrix_basis = _basis(bone, sample)
                _key(pose_bone, frame)
        yield copy
    finally:
        bpy.data.objects.remove(copy)
        bpy.data.actions.remove(action)


def _key(pose_bone, frame):
    pose_bone.keyframe_insert("location", frame=frame)
    rotation = {"QUATERNION": "rotation_quaternion", "AXIS_ANGLE": "rotation_axis_angle"}.get(pose_bone.rotation_mode, "rotation_euler")
    pose_bone.keyframe_insert(rotation, frame=frame)
    pose_bone.keyframe_insert("scale", frame=frame)


def export_poses(context, armature, filepath):
    """Writes the library poses as frames 0 to N-1; a single pose is written twice, since Unreal needs two frames."""
    samples = sample_poses(context, armature)
    if len(samples) == 1:
        samples.append(samples[0])
    with baked_copy(context, armature, samples) as copy:
        export.export_animation_fbx(context, copy, filepath, 0, len(samples) - 1)
    return len(samples)
