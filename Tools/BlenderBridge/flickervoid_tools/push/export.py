"""Exports one asset group to FBX with the locked preset, restoring the scene afterwards."""

import os
import tempfile

import bpy

from .. import scene_data

UNREAL_ROOT_NODE = "Armature"


def temp_fbx_path(asset_path):
    folder = os.path.join(tempfile.gettempdir(), "FVTools")
    os.makedirs(folder, exist_ok=True)
    return os.path.join(folder, asset_path.rstrip("/").rpartition("/")[2] + ".fbx").replace("\\", "/")


def export_fbx(context, armature, meshes, filepath):
    view_layer = context.view_layer
    selection = list(context.selected_objects)
    active = view_layer.objects.active
    renamed = []
    disabled = []
    pose_position = armature.data.pose_position if armature else None

    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    try:
        for obj in selection:
            obj.select_set(False)
        for obj in [armature, *meshes] if armature else meshes:
            obj.select_set(True)
        view_layer.objects.active = armature or meshes[0]

        # Multires stays at its base level so shape keys and vertex order match the source mesh.
        for mesh in meshes:
            for modifier in mesh.modifiers:
                if modifier.type == "MULTIRES" and (modifier.show_viewport or modifier.show_render):
                    disabled.append((modifier, modifier.show_viewport, modifier.show_render))
                    modifier.show_viewport = modifier.show_render = False

        if armature:
            armature.data.pose_position = "REST"
            # Unreal drops a skeleton root node named Armature, so the top bone becomes the root bone.
            if scene_data.top_bone(armature) == "root" and armature.name != UNREAL_ROOT_NODE:
                other = bpy.data.objects.get(UNREAL_ROOT_NODE)
                if other:
                    renamed.append((other, other.name))
                    other.name = UNREAL_ROOT_NODE + "_fv_tmp"
                renamed.insert(0, (armature, armature.name))
                armature.name = UNREAL_ROOT_NODE

        bpy.ops.export_scene.fbx(
            filepath=filepath,
            use_selection=True,
            object_types={"ARMATURE", "MESH"} if armature else {"MESH"},
            use_mesh_modifiers=True,
            use_mesh_modifiers_render=False,
            mesh_smooth_type="FACE",
            use_tspace=True,
            use_triangles=False,
            use_custom_props=False,
            add_leaf_bones=False,
            primary_bone_axis="Y",
            secondary_bone_axis="X",
            use_armature_deform_only=True,
            armature_nodetype="NULL",
            bake_anim=False,
            apply_unit_scale=True,
            apply_scale_options="FBX_SCALE_NONE",
            global_scale=1.0,
            axis_forward="-Z",
            axis_up="Y",
            path_mode="AUTO",
            embed_textures=False,
        )
    finally:
        for obj, name in renamed:
            obj.name = name
        for modifier, viewport, render in disabled:
            modifier.show_viewport, modifier.show_render = viewport, render
        if armature:
            armature.data.pose_position = pose_position
        for obj in context.selected_objects:
            obj.select_set(False)
        for obj in selection:
            obj.select_set(True)
        view_layer.objects.active = active
