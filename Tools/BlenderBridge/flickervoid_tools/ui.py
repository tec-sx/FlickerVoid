import bpy

from . import preferences
from .push.operators import FV_OT_push, FV_OT_test_connection, FV_OT_use_selected_asset
from .meatbag_factory import sculpt
from .meatbag_factory.contract.operators import MEATBAG_OT_check_contract, MEATBAG_UL_issues
from .meatbag_factory.template import process
from .meatbag_factory.template.operators import (MEATBAG_OT_edit_template, MEATBAG_OT_make_template,
                                                 MEATBAG_OT_process_template, MEATBAG_OT_remove_previous,
                                                 MEATBAG_OT_template_done, cached_state)
from .meatbag_factory.operators import (MEATBAG_OT_pose_add, MEATBAG_OT_pose_affected_add,
                                        MEATBAG_OT_pose_affected_remove, MEATBAG_OT_pose_bones_from_selection,
                                        MEATBAG_OT_pose_move, MEATBAG_OT_pose_remove, MEATBAG_OT_pose_rest,
                                        MEATBAG_OT_pose_update, MEATBAG_OT_pose_update_control,
                                        MEATBAG_OT_sculpt_back, MEATBAG_OT_sculpt_pose, MEATBAG_OT_sculpt_shape,
                                        MEATBAG_OT_send_poses, MEATBAG_OT_solve_correctives,
                                        MEATBAG_OT_use_selected_pose_asset, MEATBAG_UL_affected_bones,
                                        MEATBAG_UL_pose_results, MEATBAG_UL_poses, sculpting)


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


class _MeatbagSubpanel:
    bl_parent_id = "MEATBAG_PT_factory"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "FlickerVoid"


_TEMPLATE_STATES = {
    process.UP_TO_DATE: ("Up to date", "CHECKMARK"),
    process.CHANGED: ("Template changed: processed when used", "INFO"),
    process.TOPOLOGY: ("Topology changed: press Process", "ERROR"),
    process.EDITING: ("Editing the template", "EDITMODE_HLT"),
}


class MEATBAG_PT_body(_MeatbagSubpanel, bpy.types.Panel):
    bl_label = "Body"
    bl_idname = "MEATBAG_PT_body"

    def draw(self, context):
        layout = self.layout
        s = context.scene.meatbag
        layout.prop(s, "mesh")
        body = s.mesh
        if body is None:
            return
        template = process.template_of(body)
        if template is None:
            if process.mirror_modifier(body):
                layout.label(text="A half mesh: make it the template", icon="MOD_MIRROR")
                layout.operator(MEATBAG_OT_make_template.bl_idname, icon="DUPLICATE")
            return
        text, icon = _TEMPLATE_STATES[cached_state(body)]
        layout.label(text=f"{template.name}: {text}", icon=icon)
        if process.editing(template):
            layout.operator(MEATBAG_OT_template_done.bl_idname, icon="CHECKMARK", depress=True)
        else:
            row = layout.row(align=True)
            row.operator(MEATBAG_OT_edit_template.bl_idname, icon="EDITMODE_HLT")
            row.operator(MEATBAG_OT_process_template.bl_idname, icon="FILE_REFRESH")
        previous = process.previous_of(body)
        if previous is not None:
            row = layout.row(align=True)
            row.label(text=f"Kept: {previous.name}", icon="GHOST_ENABLED")
            row.operator(MEATBAG_OT_remove_previous.bl_idname, text="", icon="TRASH")


class MEATBAG_PT_poses(_MeatbagSubpanel, bpy.types.Panel):
    bl_label = "Poses"
    bl_idname = "MEATBAG_PT_poses"

    def draw(self, context):
        layout = self.layout
        s = context.scene.meatbag
        layout.prop(s, "armature")
        if not s.armature:
            return
        action = s.armature.meatbag_library
        row = layout.row()
        if action:
            row.template_list(MEATBAG_UL_poses.__name__, "", action, "meatbag_poses", action, "meatbag_pose_index", rows=6)
        else:
            row.label(text="No poses yet")
        column = row.column(align=True)
        column.operator(MEATBAG_OT_pose_add.bl_idname, text="", icon="ADD").rest = False
        column.operator(MEATBAG_OT_pose_remove.bl_idname, text="", icon="REMOVE")
        column.separator()
        column.operator(MEATBAG_OT_pose_move.bl_idname, text="", icon="TRIA_UP").direction = "UP"
        column.operator(MEATBAG_OT_pose_move.bl_idname, text="", icon="TRIA_DOWN").direction = "DOWN"

        row = layout.row(align=True)
        row.operator(MEATBAG_OT_pose_add.bl_idname, text="Add Rest Pose", icon="ARMATURE_DATA").rest = True
        row = layout.row(align=True)
        row.prop(s, "blend_mode", expand=True)
        row.operator(MEATBAG_OT_pose_rest.bl_idname, text="", icon="LOOP_BACK")
        if action and action.meatbag_poses:
            self.draw_active_pose(layout, s.armature, s.mesh, action.meatbag_poses[action.meatbag_pose_index])

    @staticmethod
    def draw_active_pose(layout, armature, mesh, pose):
        box = layout.box()
        row = box.row()
        row.label(text=pose.name, icon="POSE_HLT")
        keys = mesh.data.shape_keys if mesh else None
        key = keys.key_blocks.get(sculpt.shape_name(pose.name)) if keys else None
        if key:
            row.prop(key, "mute", text="Blend Shape", invert_checkbox=True)
        box.prop_search(pose, "control_bone", armature.data, "bones", text="Control")
        box.operator(MEATBAG_OT_pose_update_control.bl_idname, icon="FILE_REFRESH")
        box.label(text="Affected Bones")
        row = box.row()
        row.template_list(MEATBAG_UL_affected_bones.__name__, "", pose, "affected_bones", pose, "affected_index", rows=4)
        column = row.column(align=True)
        column.operator(MEATBAG_OT_pose_affected_add.bl_idname, text="", icon="ADD")
        column.operator(MEATBAG_OT_pose_affected_remove.bl_idname, text="", icon="REMOVE")
        box.operator(MEATBAG_OT_pose_bones_from_selection.bl_idname, icon="RESTRICT_SELECT_OFF")
        box.operator(MEATBAG_OT_pose_update.bl_idname, text="Update Affected Bones", icon="FILE_REFRESH")


class MEATBAG_PT_correctives(_MeatbagSubpanel, bpy.types.Panel):
    bl_label = "Correctives"
    bl_idname = "MEATBAG_PT_correctives"

    def draw(self, context):
        layout = self.layout
        s = context.scene.meatbag
        layout.use_property_split = True
        layout.use_property_decorate = False
        if s.armature:
            layout.prop_search(s, "helper_collection", s.armature.data, "collections_all")
        row = layout.row(align=True)
        if sculpting(context):
            row.operator(MEATBAG_OT_sculpt_back.bl_idname, icon="LOOP_BACK")
        else:
            row.operator(MEATBAG_OT_sculpt_shape.bl_idname, icon="SHAPEKEY_DATA")
            row.operator(MEATBAG_OT_sculpt_pose.bl_idname, icon="SCULPTMODE_HLT")
            if s.mesh and s.mesh.meatbag_target:
                row.operator("meatbag.rebuild_target", text="", icon="FILE_REFRESH")
        layout.prop(s, "solve_on_back")

        layout.prop(s, "solve_scope")
        layout.prop(s, "selected_helpers")
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
        layout.prop(s, "affected_only")
        layout.prop(s, "keep_source")
        layout.operator(MEATBAG_OT_send_poses.bl_idname, icon="EXPORT")


classes = (FV_PT_unreal, MEATBAG_PT_factory, MEATBAG_PT_body, MEATBAG_PT_contract, MEATBAG_PT_poses, MEATBAG_PT_correctives)
