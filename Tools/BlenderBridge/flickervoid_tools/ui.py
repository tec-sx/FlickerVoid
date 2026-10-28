import bpy

from . import preferences
from .push.operators import FV_OT_push, FV_OT_test_connection, FV_OT_use_selected_asset
from .validate.operators import FV_OT_validate, FV_UL_issues


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


class FV_PT_contract(bpy.types.Panel):
    bl_label = "Contract"
    bl_idname = "FV_PT_contract"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"

    def draw(self, context):
        layout = self.layout
        layout.operator(FV_OT_validate.bl_idname, icon="CHECKMARK")
        wm = context.window_manager
        if wm.fv_issues:
            layout.template_list(FV_UL_issues.__name__, "", wm, "fv_issues", wm, "fv_issue_index", rows=6)


classes = (FV_PT_unreal, FV_PT_contract)
