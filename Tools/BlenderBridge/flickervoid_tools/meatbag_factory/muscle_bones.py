"""Muscle bones: skin bones that follow the muscles, so the body deforms like the muscle skin without a solve.

Every muscle strand gets a belly bone (`mb_`, deforming, child of the origin bone, at the belly, aimed at the
insertion) and a target bone (`mt_`, not deforming, child of the insertion bone, at the insertion). The belly bone
stretches to the target with volume kept, so it shortens and thickens as the muscle does. Knees and elbows get a half
bone (`jh_`, turns halfway with the child bone and keeps the fold's volume) and a cap bone (`jc_`, rigid on the child
bone at the kneecap or elbow tip). Unreal repeats the same steps in a Control Rig (`rig_definition`), so the bones
work in every pose; the pose library bakes them for a Pose Asset too.

Weights are projected from the muscles: highest over a muscle's belly, fading toward its edges and tendons, and
taken out of the vertex's other weights so the total stays 1.
"""

import bpy
import numpy as np
from mathutils import Vector

from . import anatomy, muscles, spindle

BELLY = "mb_"
TARGET = "mt_"
HALF = "jh_"
CAP = "jc_"
OWNED = (BELLY, HALF, CAP)
COLLECTION = "Muscle Bones"


class MuscleBoneError(RuntimeError):
    pass


def _owned(name):
    return name.startswith(OWNED) or name.startswith(TARGET)


def _smoothstep(x):
    x = np.clip(x, 0.0, 1.0)
    return x * x * (3.0 - 2.0 * x)


def _strand_name(muscle):
    return muscle.name.removeprefix(muscles.PREFIX)


def _joint_points(armature, fitter, joint):
    bones = armature.data.bones
    child = bones[joint.child]
    direction = (child.tail_local - child.head_local).normalized()
    _, cap = fitter.point(anatomy.Attachment(joint.child, 0.0, joint.cap, 0.3))
    return child.head_local.copy(), direction, cap, child.length


def _mirror_joints(joints, sides):
    for joint in joints:
        yield joint
        name = sides.mirror_name(joint.name)
        parent, child = sides.mirror_bone(joint.parent), sides.mirror_bone(joint.child)
        if name and parent and child:
            yield anatomy.Joint(name, parent, child, joint.cap, joint.half, joint.band, joint.cap_size)


def build(context, armature, skin, sides, joints=anatomy.JOINTS):
    """Makes (or remakes) the muscle and joint bones of every muscle pushing `skin`, with their constraints."""
    strands = muscles.muscles_of(context.scene, skin)
    if not strands:
        raise MuscleBoneError("No muscles yet; generate or add muscles first")
    fitter = muscles.Fitter(armature, skin)
    joints = [joint for joint in _mirror_joints(joints, sides)
              if joint.parent in armature.data.bones and joint.child in armature.data.bones]
    plan = []
    for muscle in strands:
        settings = muscle.meatbag_muscle
        origin_anchor, insertion_anchor = muscles.anchors(muscle)
        origin = muscles.anchor_point(origin_anchor)
        insertion = muscles.anchor_point(insertion_anchor)
        belly = origin.lerp(insertion, settings.belly)
        plan.append((_strand_name(muscle), settings, belly, insertion))
    joint_plan = [(joint, *_joint_points(armature, fitter, joint)) for joint in joints]

    active, mode = context.view_layer.objects.active, context.mode
    if mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    context.view_layer.objects.active = armature
    armature.hide_set(False)
    bpy.ops.object.mode_set(mode="EDIT")
    try:
        edit_bones = armature.data.edit_bones
        for bone in [bone for bone in edit_bones if _owned(bone.name)]:
            edit_bones.remove(bone)
        for name, settings, belly, insertion in plan:
            aim = insertion - belly
            size = max(aim.length * 0.25, 1e-3)
            target = edit_bones.new(TARGET + name)
            target.head, target.tail = insertion, insertion + aim.normalized() * size * 0.4
            target.parent = edit_bones[settings.insertion_bone]
            target.use_deform = False
            bone = edit_bones.new(BELLY + name)
            bone.head, bone.tail = belly, belly + aim.normalized() * size
            bone.parent = edit_bones[settings.origin_bone]
        for joint, point, direction, cap, length in joint_plan:
            child = edit_bones[joint.child]
            half = edit_bones.new(HALF + joint.name)
            half.head, half.tail = point, point + direction * length * 0.25
            half.roll = child.roll
            half.parent = edit_bones[joint.parent]
            tip = edit_bones.new(CAP + joint.name)
            tip.head, tip.tail = cap, cap + direction * length * joint.cap_size
            tip.parent = child
    finally:
        bpy.ops.object.mode_set(mode="OBJECT")
        context.view_layer.objects.active = active

    data = armature.data
    collection = data.collections.get(COLLECTION) or data.collections.new(COLLECTION)
    for bone in data.bones:
        if _owned(bone.name):
            collection.assign(bone)
    for name, settings, belly, insertion in plan:
        pose_bone = armature.pose.bones[BELLY + name]
        stretch = pose_bone.constraints.new("STRETCH_TO")
        stretch.target, stretch.subtarget = armature, TARGET + name
        stretch.rest_length = (insertion - belly).length
        stretch.volume = "VOLUME_XZX"
        stretch.bulge = settings.volume
    for joint, *_ in joint_plan:
        copy = armature.pose.bones[HALF + joint.name].constraints.new("COPY_ROTATION")
        copy.target, copy.subtarget = armature, joint.child
        copy.influence = joint.half
    return [name for name, *_ in plan], [joint.name for joint, *_ in joint_plan]


def remove(context, armature, mesh=None):
    """Removes the muscle and joint bones and gives their weights back to the other bones."""
    if mesh is not None:
        strip_weights(mesh)
    active = context.view_layer.objects.active
    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="EDIT")
    try:
        edit_bones = armature.data.edit_bones
        for bone in [bone for bone in edit_bones if _owned(bone.name)]:
            edit_bones.remove(bone)
    finally:
        bpy.ops.object.mode_set(mode="OBJECT")
        context.view_layer.objects.active = active
    collection = armature.data.collections.get(COLLECTION)
    if collection is not None:
        armature.data.collections.remove(collection)


def _read_weights(mesh):
    names = [group.name for group in mesh.vertex_groups]
    weights = np.zeros((len(mesh.data.vertices), len(names)))
    for vertex in mesh.data.vertices:
        for element in vertex.groups:
            weights[vertex.index, element.group] = element.weight
    return names, weights


def _write_group(mesh, name, column):
    group = mesh.vertex_groups.get(name) or mesh.vertex_groups.new(name=name)
    group.remove(list(range(len(column))))
    for index in np.flatnonzero(column > 1e-4):
        group.add([int(index)], float(column[index]), "REPLACE")


def strip_weights(mesh):
    """Removes the muscle and joint bone groups and scales the other weights back up to what they were."""
    names, weights = _read_weights(mesh)
    owned = [i for i, name in enumerate(names) if name.startswith(OWNED)]
    if not owned:
        return
    taken = weights[:, owned].sum(axis=1)
    keep = [i for i in range(len(names)) if i not in owned]
    scale = np.where(taken < 1.0 - 1e-6, 1.0 / np.maximum(1.0 - taken, 1e-6), 1.0)
    for i in keep:
        if weights[:, i].any():
            _write_group(mesh, names[i], weights[:, i] * scale)
    for i in sorted(owned, reverse=True):
        mesh.vertex_groups.remove(mesh.vertex_groups[names[i]])


def _ring_radii(muscle):
    """Mean radius of each ring of the muscle's mesh (tips included), by the ring's fraction along the muscle."""
    settings = muscle.meatbag_muscle
    co = np.array([vertex.co[:] for vertex in muscle.data.vertices])
    inner = co[1:-1].reshape(-1, 12, 3)
    along = np.concatenate(([0.0], inner[:, 0, 1] / max(settings.length, 1e-6), [1.0]))
    radii = np.concatenate(([0.0], np.hypot(inner[:, :, 0], inner[:, :, 2]).mean(axis=1), [0.0]))
    return along, radii


def _bone_mask(names, weights, bones):
    columns = [names.index(bone) for bone in bones if bone in names]
    return np.clip(weights[:, columns].sum(axis=1), 0.0, 1.0) if columns else np.zeros(len(weights))


def project_weights(context, armature, mesh, skin, strength=0.5, joint_strength=0.6, cap_strength=0.7, spread=1.0,
                    total=0.8, joints=anatomy.JOINTS, sides=None):
    """Weights the muscle and joint bones on `mesh` from the muscles pushing `skin` and the joint helpers.

    A muscle bone's weight is `strength` over the muscle's belly, fading over `spread` belly radii from its surface
    and toward its tendons, only where the muscle's own bones already move the skin. Together the new bones take at
    most `total` of a vertex; the other weights shrink to keep the sum at 1."""
    strip_weights(mesh)
    names, weights = _read_weights(mesh)
    to_armature = np.array(armature.matrix_world.inverted() @ mesh.matrix_world)
    co = np.array([vertex.co[:] for vertex in mesh.data.vertices])
    points = co @ to_armature[:3, :3].T + to_armature[:3, 3]
    columns, values = [], []

    for muscle in muscles.muscles_of(context.scene, skin):
        settings = muscle.meatbag_muscle
        name = BELLY + _strand_name(muscle)
        if name not in armature.data.bones:
            continue
        origin_anchor, insertion_anchor = muscles.anchors(muscle)
        origin = np.array(muscles.anchor_point(origin_anchor))
        insertion = np.array(muscles.anchor_point(insertion_anchor))
        axis = insertion - origin
        length = max(np.linalg.norm(axis), 1e-6)
        axis = axis / length
        along = (points - origin) @ axis / length
        radial = np.linalg.norm((points - origin) - np.outer(along * length, axis), axis=1)
        ring_along, ring_radii = _ring_radii(muscle)
        radius = np.interp(np.clip(along, 0.0, 1.0), ring_along, ring_radii)
        belly = max(ring_radii.max(), 1e-6)
        outside = np.maximum(radial - radius, 0.0)
        hooked = [modifier.object.parent_bone for modifier in muscle.modifiers
                  if modifier.type == "HOOK" and modifier.object is not None]
        mask = _bone_mask(names, weights, (settings.origin_bone, settings.insertion_bone, *hooked))
        taper = spindle.radius_profile(np.clip(along, 0.0, 1.0), settings.belly, 0.0)
        inside = (along > 0.0) & (along < 1.0)
        value = strength * taper * _smoothstep(1.0 - outside / (spread * belly)) * mask * inside
        columns.append(name)
        values.append(value)

    bones = armature.data.bones
    for joint in (_mirror_joints(joints, sides) if sides else joints):
        if HALF + joint.name not in bones:
            continue
        child, parent = bones[joint.child], bones[joint.parent]
        point = np.array(child.head_local)
        direction = np.array((child.tail_local - child.head_local).normalized())
        mask = _bone_mask(names, weights, (joint.parent, joint.child))
        band = joint.band * (child.length + parent.length) * 0.5
        offset = (points - point) @ direction
        columns.append(HALF + joint.name)
        values.append(joint_strength * _smoothstep(1.0 - np.abs(offset) / band) * mask)
        cap = np.array(bones[CAP + joint.name].head_local)
        distance = np.linalg.norm(points - cap, axis=1)
        columns.append(CAP + joint.name)
        values.append(cap_strength * _smoothstep(1.0 - distance / (joint.cap_size * child.length)) * mask)

    if not columns:
        raise MuscleBoneError("No muscle bones; build them first")
    added = np.stack(values, axis=1)
    sums = added.sum(axis=1)
    added *= np.where(sums > total, total / np.maximum(sums, 1e-9), 1.0)[:, None]
    taken = added.sum(axis=1)
    for i, name in enumerate(names):
        if weights[:, i].any() and not _owned(name):
            _write_group(mesh, name, weights[:, i] * (1.0 - taken))
    for name, column in zip(columns, added.T):
        _write_group(mesh, name, column)
    return columns


def rig_definition(armature):
    """What Unreal's FV Muscle Bones node needs: each belly bone with its target and volume, each half bone with
    the bone it follows and how far."""
    stretch, follow = [], []
    for pose_bone in armature.pose.bones:
        for constraint in pose_bone.constraints:
            if pose_bone.name.startswith(BELLY) and constraint.type == "STRETCH_TO":
                stretch.append({"bone": pose_bone.name, "target": constraint.subtarget, "volume": constraint.bulge})
            elif pose_bone.name.startswith(HALF) and constraint.type == "COPY_ROTATION":
                follow.append({"bone": pose_bone.name, "source": constraint.subtarget, "weight": constraint.influence})
    return {"stretch": stretch, "follow": follow}
