"""Weight fixes for the Validate panel, written straight into the vertex groups so they also reach hidden templates.

A body made from a template gets its weights from the template, so the fixes go there and the body is processed after.
"""

import bpy

from ..template import process
from ... import scene_data
from ...core import rules


def target(mesh):
    """The mesh whose weights are authored: the template when the body has one."""
    return process.template_of(mesh) or mesh


def _bone_names(mesh):
    armature = scene_data.armature_of(mesh)
    return {name for name, _ in scene_data.deform_bones(armature)} if armature else set()


def _deform_weights(obj, bone_names):
    """Per vertex, {group index: weight} for the deform bones' groups."""
    deform = {group.index for group in obj.vertex_groups if group.name in bone_names}
    return [{element.group: element.weight for element in vertex.groups if element.group in deform}
            for vertex in obj.data.vertices]


def limit_total(mesh, limit):
    """Drops each vertex's smallest deform influences beyond the limit; returns how many vertices changed."""
    obj = target(mesh)
    groups = obj.vertex_groups
    changed = 0
    for index, weights in enumerate(_deform_weights(obj, _bone_names(mesh))):
        nonzero = {group: weight for group, weight in weights.items() if weight > 0.0}
        dropped = rules.limited(nonzero, limit)
        dropped += [group for group, weight in weights.items() if weight <= 0.0]
        for group in dropped:
            groups[group].remove([index])
        changed += bool(rules.limited(nonzero, limit))
    return changed


def normalize(mesh):
    """Scales each vertex's deform weights to sum to 1; returns how many vertices changed."""
    obj = target(mesh)
    groups = obj.vertex_groups
    changed = 0
    for index, weights in enumerate(_deform_weights(obj, _bone_names(mesh))):
        if abs(sum(weights.values()) - 1.0) <= rules.WEIGHT_SUM_TOLERANCE or not any(weights.values()):
            continue
        for group, weight in rules.normalized(weights).items():
            groups[group].add([index], weight, "REPLACE")
        changed += 1
    return changed


def _kept(obj):
    """Groups something other than the skin uses: modifiers (masks, Mirror, Shrinkwrap...) and shape keys."""
    kept = set()
    for modifier in obj.modifiers:
        for name in dir(modifier):
            if "vertex_group" in name and isinstance(value := getattr(modifier, name, None), str) and value:
                kept.add(value)
    if obj.data.shape_keys:
        kept |= {key.vertex_group for key in obj.data.shape_keys.key_blocks if key.vertex_group}
    return kept


def remove_unused(mesh, sides):
    """Removes empty vertex groups and the ones matching no deform bone; returns their names."""
    obj = target(mesh)
    weighted = set()
    for vertex in obj.data.vertices:
        weighted |= {obj.vertex_groups[element.group].name for element in vertex.groups if element.weight > 0.0}
    names = [group.name for group in obj.vertex_groups]
    unused = rules.unused_groups(names, _bone_names(mesh), weighted, _kept(obj), sides)
    for name in unused:
        obj.vertex_groups.remove(obj.vertex_groups[name])
    return unused


def remove_root_bone(context, armature):
    """Deletes the root bone, leaving its children at the top. Its vertex group is left for Remove Unused Groups."""
    view_layer = context.view_layer
    active, hidden = view_layer.objects.active, armature.hide_get()
    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    armature.hide_set(False)
    view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="EDIT")
    try:
        edit_bones = armature.data.edit_bones
        root = edit_bones.get(rules.ROOT_BONE)
        if root is None:
            return False
        for child in root.children:
            child.use_connect = False
            child.parent = None
        edit_bones.remove(root)
        return True
    finally:
        bpy.ops.object.mode_set(mode="OBJECT")
        armature.hide_set(hidden)
        view_layer.objects.active = active
