import json

import bpy

from . import checks
from .. import preferences
from ..core import rules


class FVIssue(bpy.types.PropertyGroup):
    level: bpy.props.StringProperty()
    message: bpy.props.StringProperty()
    object_name: bpy.props.StringProperty()
    item_kind: bpy.props.StringProperty()
    items: bpy.props.StringProperty(description="JSON list of bone names or vertex indices")
    fix: bpy.props.StringProperty()


def store_issues(context, issues):
    stored = context.window_manager.fv_issues
    stored.clear()
    for issue in issues:
        item = stored.add()
        item.level = issue.level
        item.message = issue.message
        item.object_name = issue.object_name
        item.item_kind = issue.item_kind
        item.items = json.dumps(issue.items)
        item.fix = issue.fix
    context.window_manager.fv_issue_index = 0


def summary(issues):
    errors = sum(issue.level == rules.ERROR for issue in issues)
    warnings = len(issues) - errors
    return f"{errors} errors, {warnings} warnings" if issues else "No problems found"


class FV_OT_validate(bpy.types.Operator):
    bl_idname = "fv.validate"
    bl_label = "Check Contract"
    bl_description = "Check the selected armatures and meshes against the character contract"

    @classmethod
    def poll(cls, context):
        return bool(context.selected_objects)

    def execute(self, context):
        issues = checks.run(context, context.selected_objects)
        store_issues(context, issues)
        self.report({"WARNING"} if issues else {"INFO"}, summary(issues))
        return {"FINISHED"}


class FV_OT_select_issue(bpy.types.Operator):
    bl_idname = "fv.select_issue"
    bl_label = "Select"
    bl_description = "Select the object, bones or vertices this problem is about"
    bl_options = {"REGISTER", "UNDO"}

    index: bpy.props.IntProperty()

    def execute(self, context):
        issue = context.window_manager.fv_issues[self.index]
        obj = context.scene.objects.get(issue.object_name)
        if not obj:
            self.report({"WARNING"}, "Nothing to select")
            return {"CANCELLED"}

        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        for other in context.selected_objects:
            other.select_set(False)
        obj.select_set(True)
        context.view_layer.objects.active = obj

        items = json.loads(issue.items or "[]")
        if issue.item_kind == rules.BONES and obj.type == "ARMATURE":
            names = set(items)
            for pose_bone in obj.pose.bones:
                # Blender 5 keeps pose selection on the pose bone, 4.x on the bone.
                target = pose_bone if hasattr(pose_bone, "select") else pose_bone.bone
                target.select = pose_bone.name in names
            bpy.ops.object.mode_set(mode="POSE")
        elif issue.item_kind == rules.VERTICES and obj.type == "MESH":
            indices = set(items)
            mesh = obj.data
            for polygon in mesh.polygons:
                polygon.select = False
            for edge in mesh.edges:
                edge.select = False
            for vertex in mesh.vertices:
                vertex.select = vertex.index in indices
            bpy.ops.object.mode_set(mode="EDIT")
            bpy.ops.mesh.select_mode(type="VERT")
        return {"FINISHED"}


class FV_OT_fix_issue(bpy.types.Operator):
    bl_idname = "fv.fix_issue"
    bl_label = "Fix"
    bl_description = "Apply the safe fix for this problem, then check again"
    bl_options = {"REGISTER", "UNDO"}

    index: bpy.props.IntProperty()

    def execute(self, context):
        issue = context.window_manager.fv_issues[self.index]
        obj = context.scene.objects.get(issue.object_name)
        if not obj or obj.type != "MESH" or not issue.fix:
            return {"CANCELLED"}

        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        max_influences = preferences.contract(context).max_influences
        selection = list(context.selected_objects)
        active = context.view_layer.objects.active
        # The vertex group operators act on the active object only while it is selected.
        for other in selection:
            other.select_set(False)
        obj.select_set(True)
        context.view_layer.objects.active = obj
        try:
            if issue.fix == rules.FIX_LIMIT_INFLUENCES:
                bpy.ops.object.vertex_group_limit_total(group_select_mode="BONE_DEFORM", limit=max_influences)
            bpy.ops.object.vertex_group_normalize_all(group_select_mode="BONE_DEFORM", lock_active=False)
        finally:
            obj.select_set(False)
            for other in selection:
                other.select_set(True)
            context.view_layer.objects.active = active

        issues = checks.run(context, selection or [obj])
        store_issues(context, issues)
        self.report({"INFO"}, f"Fixed. {summary(issues)}")
        return {"FINISHED"}


class FV_UL_issues(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index=0, flt_flag=0):
        row = layout.row(align=True)
        row.label(text=f"{item.object_name}: {item.message}" if item.object_name else item.message,
                  icon="ERROR" if item.level == rules.ERROR else "INFO")
        if item.item_kind != rules.NONE or item.object_name:
            row.operator(FV_OT_select_issue.bl_idname, text="", icon="RESTRICT_SELECT_OFF").index = index
        if item.fix:
            row.operator(FV_OT_fix_issue.bl_idname, text="", icon="TOOL_SETTINGS").index = index


classes = (FVIssue, FV_OT_validate, FV_OT_select_issue, FV_OT_fix_issue, FV_UL_issues)


def register_properties():
    bpy.types.WindowManager.fv_issues = bpy.props.CollectionProperty(type=FVIssue)
    bpy.types.WindowManager.fv_issue_index = bpy.props.IntProperty()


def unregister_properties():
    del bpy.types.WindowManager.fv_issues
    del bpy.types.WindowManager.fv_issue_index
