import bpy

from . import preferences
from .push.operators import FV_OT_push, FV_OT_test_connection, FV_OT_use_selected_asset
from .meatbag_factory.contract.operators import MEATBAG_OT_check_contract, MEATBAG_UL_issues
from .meatbag_factory.operators import (MEATBAG_OT_send_poses, MEATBAG_OT_solve_correctives,
                                        MEATBAG_OT_use_selected_pose_asset, MEATBAG_UL_pose_results)


class FV_PT_unreal(bpy.types.Panel):
    bl_label = "Unreal"
    bl_idname = "FV_PT_unreal"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"

    def draw(self, context):
        layout = self.layout
        if not preferences.get(context).project_dir:
            layout.label(text="Set the Unreal project in the add-on preferences", icon="INFO")
        layout.operator(FV_OT_test_connection.bl_idname, icon="LINKED")

        obj = context.active_object
        if obj and obj.type == "MESH":
            box = layout.box()
            box.label(text=obj.name, icon="MESH_DATA")
            box.prop(obj, "fv_ue_asset", text="Asset")
            box.prop(obj, "fv_ue_skeleton", text="Skeleton")
        layout.operator(FV_OT_use_selected_asset.bl_idname, icon="EYEDROPPER")
        layout.operator(FV_OT_push.bl_idname, icon="EXPORT")


class MEATBAG_PT_factory(bpy.types.Panel):
    bl_label = "Meatbag Factory"
    bl_idname = "MEATBAG_PT_factory"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"

    def draw(self, context):
        pass


class MEATBAG_PT_contract(bpy.types.Panel):
    bl_label = "Contract"
    bl_idname = "MEATBAG_PT_contract"
    bl_parent_id = MEATBAG_PT_factory.bl_idname
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"

    def draw(self, context):
        layout = self.layout
        layout.operator(MEATBAG_OT_check_contract.bl_idname, icon="CHECKMARK")
        wm = context.window_manager
        if wm.meatbag_issues:
            layout.template_list(MEATBAG_UL_issues.__name__, "", wm, "meatbag_issues", wm, "meatbag_issue_index", rows=6)


class MEATBAG_PT_correctives(bpy.types.Panel):
    bl_label = "Correctives"
    bl_idname = "MEATBAG_PT_correctives"
    bl_parent_id = MEATBAG_PT_factory.bl_idname
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"

    def draw(self, context):
        layout = self.layout
        s = context.scene.meatbag
        layout.use_property_split = True
        layout.use_property_decorate = False
        layout.prop(s, "armature")
        if s.armature:
            layout.prop_search(s, "helper_collection", s.armature.data, "collections_all")
        layout.prop(s, "mesh")
        layout.prop(s, "target")

        column = layout.column(heading="Solve")
        column.prop(s, "allow_scale")
        column.prop(s, "solve_weights")
        column.prop(s, "iterations")
        row = column.row()
        row.enabled = s.solve_weights
        row.prop(s, "weight_rounds")
        column.prop(s, "residual_keys")
        row = column.row()
        row.enabled = s.residual_keys
        row.prop(s, "residual_threshold")
        layout.operator(MEATBAG_OT_solve_correctives.bl_idname, icon="BONE_DATA")
        if s.results:
            layout.label(text="RMS error per pose: before, after")
            layout.template_list(MEATBAG_UL_pose_results.__name__, "", s, "results", s, "result_index", rows=4)

        layout.separator()
        row = layout.row(align=True)
        row.prop(s, "pose_asset")
        row.operator(MEATBAG_OT_use_selected_pose_asset.bl_idname, text="", icon="EYEDROPPER")
        layout.prop(s, "keep_source")
        layout.operator(MEATBAG_OT_send_poses.bl_idname, icon="EXPORT")


classes = (FV_PT_unreal, MEATBAG_PT_factory, MEATBAG_PT_contract, MEATBAG_PT_correctives)
