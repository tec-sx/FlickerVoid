import json

import bpy

from . import checks, skeleton, weights
from ... import preferences, scene_data
from ...core import rules


class MeatbagIssue(bpy.types.PropertyGroup):
    level: bpy.props.StringProperty()
    message: bpy.props.StringProperty()
    object_name: bpy.props.StringProperty()
    item_kind: bpy.props.StringProperty()
    items: bpy.props.StringProperty(description="JSON list of bone names or vertex indices")
    fix: bpy.props.StringProperty()


def store_issues(context, issues):
    stored = context.window_manager.meatbag_issues
    stored.clear()
    for issue in issues:
        item = stored.add()
        item.level = issue.level
        item.message = issue.message
        item.object_name = issue.object_name
        item.item_kind = issue.item_kind
        item.items = json.dumps(issue.items)
        item.fix = issue.fix
    context.window_manager.meatbag_issue_index = 0


def summary(issues):
    errors = sum(issue.level == rules.ERROR for issue in issues)
    warnings = len(issues) - errors
    return f"{errors} errors, {warnings} warnings" if issues else "No problems found"


class MEATBAG_OT_validate(bpy.types.Operator):
    bl_idname = "meatbag.validate"
    bl_label = "Validate"
    bl_description = "Check the selected armatures and meshes against the character contract"

    @classmethod
    def poll(cls, context):
        return bool(context.selected_objects)

    def execute(self, context):
        issues = checks.run(context, context.selected_objects)
        store_issues(context, issues)
        self.report({"WARNING"} if issues else {"INFO"}, summary(issues))
        return {"FINISHED"}


class MEATBAG_OT_select_issue(bpy.types.Operator):
    bl_idname = "meatbag.select_issue"
    bl_label = "Select"
    bl_description = "Select the object, bones or vertices this problem is about"
    bl_options = {"REGISTER", "UNDO"}

    index: bpy.props.IntProperty()

    def execute(self, context):
        issue = context.window_manager.meatbag_issues[self.index]
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


class MEATBAG_OT_fix_issue(bpy.types.Operator):
    bl_idname = "meatbag.fix_issue"
    bl_label = "Fix"
    bl_description = "Apply the safe fix for this problem, then check again"
    bl_options = {"REGISTER", "UNDO"}

    index: bpy.props.IntProperty()

    def execute(self, context):
        issue = context.window_manager.meatbag_issues[self.index]
        obj = context.scene.objects.get(issue.object_name)
        if not obj or not issue.fix:
            return {"CANCELLED"}
        selection = list(context.selected_objects) or [obj]
        if issue.fix == rules.FIX_REMOVE_ROOT:
            if obj.type != "ARMATURE":
                return {"CANCELLED"}
            weights.remove_root_bone(context, obj)
            text = f"Removed {rules.ROOT_BONE} from {obj.name}"
        else:
            fix = _ISSUE_FIXES.get(issue.fix)
            if obj.type != "MESH" or fix is None:
                return {"CANCELLED"}
            text = _apply(self, context, fix, [obj])
            if text is None:
                return {"CANCELLED"}

        issues = checks.run(context, selection)
        store_issues(context, issues)
        self.report({"INFO"}, f"{text}. {summary(issues)}")
        return {"FINISHED"}


def _limit(context, meshes):
    limit = preferences.contract(context).max_influences
    changed = sum(weights.limit_total(mesh, limit) for mesh in meshes)
    return f"Limited {changed} vertices to {limit} influences"


def _normalize(context, meshes):
    changed = sum(weights.normalize(mesh) for mesh in meshes)
    return f"Normalized {changed} vertices"


def _remove_groups(context, meshes):
    sides = preferences.contract(context).sides
    removed = [name for mesh in meshes for name in weights.remove_unused(mesh, sides)]
    return f"Removed {len(removed)} vertex groups: {rules.preview(removed)}" if removed else "No unused vertex groups"


def _limit_and_normalize(context, meshes):
    return f"{_limit(context, meshes)}; {_normalize(context, meshes)}"


_WEIGHT_FIXES = {
    rules.FIX_REMOVE_GROUPS: _remove_groups,
    rules.FIX_LIMIT_INFLUENCES: _limit,
    rules.FIX_NORMALIZE: _normalize,
}
_ISSUE_FIXES = {**_WEIGHT_FIXES, rules.FIX_LIMIT_INFLUENCES: _limit_and_normalize}


def _apply(operator, context, fix, meshes):
    """Runs a weight fix on the meshes (their templates when they have one), then processes the changed bodies."""
    from ..template.operators import ensure_current

    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    text = fix(context, meshes)
    for mesh in meshes:
        if weights.target(mesh) is not mesh and not ensure_current(operator, context, mesh):
            return None
    return text


def _selected_meshes(context):
    objects = scene_data.expand_selection(context.selected_objects, context.scene)
    return [obj for obj in objects if obj.type == "MESH" and not scene_data.is_helper(obj)]


class MEATBAG_OT_fix_weights(bpy.types.Operator):
    bl_idname = "meatbag.fix_weights"
    bl_label = "Fix Weights"
    bl_description = "Fix the weights of the selected meshes (and the meshes of selected armatures), then validate again"
    bl_options = {"REGISTER", "UNDO"}

    fix: bpy.props.EnumProperty(items=(
        (rules.FIX_REMOVE_GROUPS, "Remove Unused Groups",
         "Remove empty vertex groups and the ones matching no deform bone. Groups used by modifiers or shape keys stay, "
         "and so do a template's empty counterpart groups, which its Mirror fills"),
        (rules.FIX_LIMIT_INFLUENCES, "Limit Total",
         "Keep each vertex's largest deform influences, up to Max Influences from the preferences"),
        (rules.FIX_NORMALIZE, "Normalize", "Make each vertex's deform weights sum to 1"),
    ))

    @classmethod
    def poll(cls, context):
        return bool(_selected_meshes(context))

    @classmethod
    def description(cls, context, properties):
        return next(item.description for item in cls.bl_rna.properties["fix"].enum_items if item.identifier == properties.fix)

    def execute(self, context):
        meshes = _selected_meshes(context)
        selection = list(context.selected_objects)
        text = _apply(self, context, _WEIGHT_FIXES[self.fix], meshes)
        if text is None:
            return {"CANCELLED"}
        issues = checks.run(context, selection)
        store_issues(context, issues)
        self.report({"INFO"}, f"{text}. {summary(issues)}")
        return {"FINISHED"}


def skeleton_armature(context):
    """The armature picked in Poses, else the active armature or the active mesh's armature."""
    armature = context.scene.meatbag.armature
    if armature is not None:
        return armature
    obj = context.active_object
    if obj is not None and obj.type == "ARMATURE":
        return obj
    return scene_data.armature_of(obj) if obj is not None else None


class _SkeletonOperator:
    @classmethod
    def poll(cls, context):
        return bool(context.window_manager.meat_locker_reference) and skeleton_armature(context) is not None


class MEATBAG_OT_check_skeleton(_SkeletonOperator, bpy.types.Operator):
    bl_idname = "meatbag.check_skeleton"
    bl_label = "Check Skeleton"
    bl_description = ("Compare the armature's deform bones with the Meat Locker's Reference character: missing and extra "
                      "bones, parents and rolls")

    def execute(self, context):
        armature = skeleton_armature(context)
        try:
            issues = skeleton.check(context, armature)
        except skeleton.SkeletonError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        store_issues(context, issues)
        self.report({"WARNING"} if issues else {"INFO"}, summary(issues) if issues else f"{armature.name} matches the reference")
        return {"FINISHED"}


class MEATBAG_OT_sync_skeleton(_SkeletonOperator, bpy.types.Operator):
    bl_idname = "meatbag.sync_skeleton"
    bl_label = "Sync Skeleton"
    bl_description = ("Add the Reference character's deform bones that this armature lacks, placed relative to their parent "
                      "and scaled by its length, in the same bone collections. Other differences are only reported")
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        armature = skeleton_armature(context)
        try:
            added = skeleton.sync(context, armature)
            issues = skeleton.check(context, armature)
        except skeleton.SkeletonError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        store_issues(context, issues)
        text = f"Added {len(added)} bones: {rules.preview(added)}; paint or solve their weights" if added else "No bones missing"
        if issues:
            text += f". Still different: {summary(issues)}"
        self.report({"WARNING"} if issues else {"INFO"}, text)
        return {"FINISHED"}


class MEATBAG_UL_issues(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index=0, flt_flag=0):
        row = layout.row(align=True)
        row.label(text=f"{item.object_name}: {item.message}" if item.object_name else item.message,
                  icon="ERROR" if item.level == rules.ERROR else "INFO")
        if item.item_kind != rules.NONE or item.object_name:
            row.operator(MEATBAG_OT_select_issue.bl_idname, text="", icon="RESTRICT_SELECT_OFF").index = index
        if item.fix:
            row.operator(MEATBAG_OT_fix_issue.bl_idname, text="", icon="TOOL_SETTINGS").index = index


classes = (MeatbagIssue, MEATBAG_OT_validate, MEATBAG_OT_select_issue, MEATBAG_OT_fix_issue, MEATBAG_OT_fix_weights,
           MEATBAG_OT_check_skeleton, MEATBAG_OT_sync_skeleton, MEATBAG_UL_issues)


def register_properties():
    bpy.types.WindowManager.meatbag_issues = bpy.props.CollectionProperty(type=MeatbagIssue)
    bpy.types.WindowManager.meatbag_issue_index = bpy.props.IntProperty()


def unregister_properties():
    del bpy.types.WindowManager.meatbag_issues
    del bpy.types.WindowManager.meatbag_issue_index
