import bpy

from .. import preferences, scene_data
from ..core import rules
from ..core.rules import ERROR, Issue


def run(context, objects):
    contract = preferences.contract(context)
    objects = scene_data.expand_selection(objects, context.scene)
    issues = rules.check_unit_scale(context.scene.unit_settings.scale_length, contract.unit_scale)

    armatures = {obj for obj in objects if obj.type == "ARMATURE"}
    armatures |= {arm for obj in objects if (arm := scene_data.armature_of(obj))}
    for armature in sorted(armatures, key=lambda obj: obj.name):
        issues += rules.check_armature(armature.name, scene_data.deform_bones(armature), contract)

    for mesh in (obj for obj in objects if obj.type == "MESH"):
        armature = scene_data.armature_of(mesh)
        if not armature:
            continue
        if not any(mod.type == "ARMATURE" and mod.object == armature for mod in mesh.modifiers):
            issues.append(Issue(ERROR, f"No Armature modifier pointing at {armature.name}", mesh.name))
        bone_names = {name for name, _ in scene_data.deform_bones(armature)}
        issues += rules.check_vertex_groups(mesh.name, [group.name for group in mesh.vertex_groups], bone_names)
        issues += rules.check_weights(mesh.name, scene_data.vertex_deform_weights(mesh, bone_names), contract.max_influences)
    return issues


def has_errors(issues):
    return any(issue.level == ERROR for issue in issues)
