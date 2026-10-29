import bpy

from . import preferences
from .push.operators import FV_OT_push, FV_OT_test_connection, FV_OT_use_selected_asset
from .meatbag_factory import muscles
from .meatbag_factory.muscle_operators import (MEATBAG_OT_muscles_generate, MEATBAG_OT_muscles_fit, MEATBAG_OT_muscle_add, MEATBAG_OT_muscle_mirror, MEATBAG_OT_muscle_remove,
                                               MEATBAG_OT_muscle_select, MEATBAG_OT_preview_skin)
from .meatbag_factory.contract.operators import MEATBAG_OT_check_contract, MEATBAG_UL_issues
from .meatbag_factory.operators import (MEATBAG_OT_pose_add, MEATBAG_OT_pose_move, MEATBAG_OT_pose_remove,
                                        MEATBAG_OT_pose_rest, MEATBAG_OT_pose_update, MEATBAG_OT_sculpt_back,
                                        MEATBAG_OT_sculpt_pose, MEATBAG_OT_send_poses,
                                        MEATBAG_OT_solve_correctives, MEATBAG_OT_use_selected_pose_asset,
                                        MEATBAG_UL_pose_results, MEATBAG_UL_poses)


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
        row.operator(MEATBAG_OT_pose_update.bl_idname, icon="FILE_REFRESH")
        row = layout.row(align=True)
        row.prop(s, "blend_mode", expand=True)
        row.operator(MEATBAG_OT_pose_rest.bl_idname, text="", icon="LOOP_BACK")


class MEATBAG_PT_muscles(_MeatbagSubpanel, bpy.types.Panel):
    bl_label = "Muscles"
    bl_idname = "MEATBAG_PT_muscles"

    def draw(self, context):
        layout = self.layout
        s = context.scene.meatbag
        if not s.armature or not s.mesh:
            layout.label(text="Pick the Armature (Poses) and the Mesh (Correctives)", icon="INFO")
            return
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_muscles_generate.bl_idname, icon="OUTLINER_OB_ARMATURE")
        row.operator(MEATBAG_OT_muscles_fit.bl_idname, icon="MOD_SHRINKWRAP")
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_muscle_add.bl_idname, icon="ADD")
        row.operator(MEATBAG_OT_preview_skin.bl_idname, icon="HIDE_OFF")

        view = context.scene.meatbag_muscles
        row = layout.row(align=True)
        row.prop(view, "visible", text="", icon="HIDE_OFF" if view.visible else "HIDE_ON")
        row.prop(view, "opacity", slider=True)
        row.prop(view, "solid", text="", icon="SHADING_SOLID")
        row.prop(view, "in_front", text="", icon="XRAY")

        target = s.mesh.meatbag_target
        column = layout.column(align=True)
        groups = {}
        for muscle in muscles.muscles_of(context.scene, target):
            groups.setdefault(muscle.meatbag_muscle.group or muscle.name, []).append(muscle)
        for group, strands in sorted(groups.items()):
            row = column.row(align=True)
            row.label(text=group, icon="FORCE_LENNARDJONES")
            for number, strand in enumerate(sorted(strands, key=lambda obj: obj.name), 1):
                text = str(number) if len(strands) > 1 else "Select"
                row.operator(MEATBAG_OT_muscle_select.bl_idname, text=text,
                             depress=strand == context.active_object).name = strand.name

        obj = context.active_object
        if obj and obj.type == "MESH" and obj.meatbag_muscle.skin is not None:
            settings = obj.meatbag_muscle
            box = layout.box()
            box.label(text=f"{obj.name}: {settings.origin_bone} to {settings.insertion_bone}")
            box.use_property_split = True
            for prop in ("width", "thickness", "belly", "tendon", "volume", "offset"):
                box.prop(settings, prop)
            row = box.row(align=True)
            row.operator(MEATBAG_OT_muscle_mirror.bl_idname, icon="MOD_MIRROR")
            row.operator(MEATBAG_OT_muscle_remove.bl_idname, icon="X")


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
        layout.prop(s, "mesh")
        row = layout.row(align=True)
        sculpting = s.mesh and s.mesh.meatbag_target and context.active_object == s.mesh.meatbag_target
        row.operator(MEATBAG_OT_sculpt_back.bl_idname if sculpting else MEATBAG_OT_sculpt_pose.bl_idname,
                     icon="LOOP_BACK" if sculpting else "SCULPTMODE_HLT")
        layout.prop(s, "solve_on_back")
        layout.prop(s, "target", text="Other Target")

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
        layout.prop(s, "keep_source")
        layout.operator(MEATBAG_OT_send_poses.bl_idname, icon="EXPORT")


classes = (FV_PT_unreal, MEATBAG_PT_factory, MEATBAG_PT_contract, MEATBAG_PT_poses, MEATBAG_PT_muscles, MEATBAG_PT_correctives)
