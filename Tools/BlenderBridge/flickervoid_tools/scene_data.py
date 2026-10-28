"""Gathers armatures and meshes from the Blender scene for the validator and the push."""

import bpy


def armature_of(obj):
    if obj.type != "MESH":
        return None
    for modifier in obj.modifiers:
        if modifier.type == "ARMATURE" and modifier.object:
            return modifier.object
    if obj.parent and obj.parent.type == "ARMATURE":
        return obj.parent
    return None


def deformed_meshes(armature, scene):
    return [obj for obj in scene.objects if obj.type == "MESH" and armature_of(obj) == armature]


def deform_bones(armature):
    """(name, parent) for the bones that reach Unreal: deform bones, parented to their nearest deform ancestor."""
    bones = []
    for bone in armature.data.bones:
        if not bone.use_deform:
            continue
        parent = bone.parent
        while parent and not parent.use_deform:
            parent = parent.parent
        bones.append((bone.name, parent.name if parent else None))
    return bones


def top_bone(armature):
    tops = [name for name, parent in deform_bones(armature) if parent is None]
    return tops[0] if len(tops) == 1 else None


def vertex_deform_weights(mesh, bone_names):
    groups = {group.index for group in mesh.vertex_groups if group.name in bone_names}
    return [[element.weight for element in vertex.groups if element.group in groups and element.weight > 0.0]
            for vertex in mesh.data.vertices]


def expand_selection(objects, scene):
    """The selected objects plus the meshes deformed by any selected armature."""
    result = list(dict.fromkeys(objects))
    for obj in objects:
        if obj.type == "ARMATURE":
            result.extend(mesh for mesh in deformed_meshes(obj, scene) if mesh not in result)
    return [obj for obj in result if obj.type in {"MESH", "ARMATURE"}]
