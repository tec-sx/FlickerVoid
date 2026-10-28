"""Meatbag Factory correctives: the pose library, the helper bone solve, and sending the poses to a Pose Asset in Unreal."""

import numpy as np

import bpy
from mathutils import Matrix

from . import apply, capture, library, pose_asset, sculpt, solver
from .. import preferences
from ..push import export
from ..unreal_link import link


def _is_armature(_, obj):
    return obj.type == "ARMATURE"


def _is_mesh(_, obj):
    return obj.type == "MESH"


def settings(context):
    return context.scene.meatbag


def _library_armature(context, action):
    armature = settings(context).armature
    if armature and armature.meatbag_library == action:
        return armature
    return next((obj for obj in context.scene.objects if obj.type == "ARMATURE" and obj.meatbag_library == action), None)


def show_preview(context, action):
    """Poses the library's armature as the mix of the poses' preview weights."""
    armature = _library_armature(context, action)
    if armature is None:
        return
    bones = library.bone_names(armature)
    weights = [pose.weight for pose in action.meatbag_poses]
    library.set_pose(armature, bones, library.mix(library.read(action, bones), weights, settings(context).blend_mode))


def _weight_changed(pose, context):
    show_preview(context, pose.id_data)


def _name_changed(pose, context):
    """Keeps names unique and renames the pose's sculpt target shape key with it."""
    others = {other.name for other in pose.id_data.meatbag_poses if other.as_pointer() != pose.as_pointer()}
    base = pose.name.strip() or "pose"
    name, suffix = base, 1
    while name in others:
        name, suffix = f"{base}_{suffix}", suffix + 1
    if name != pose.name:
        pose["name"] = name
    old = pose.synced_name
    target = _sculpt_target(context)
    if old and target and old != name:
        sculpt.rename_pose_key(target, old, name)
    pose["synced_name"] = name


def _sculpt_target(context):
    mesh = settings(context).mesh
    return mesh.meatbag_target if mesh else None


class MeatbagPose(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="Name", update=_name_changed,
                                   description="Pose name in Unreal, with its side token (thigh_l_raise_90)")
    weight: bpy.props.FloatProperty(name="Weight", default=0.0, soft_min=0.0, soft_max=1.0, update=_weight_changed,
                                    description="Preview weight; the viewport shows the mix of all weights")
    locked: bpy.props.BoolProperty(name="Locked", description="Solve leaves this pose's helpers as they are, for hand-tuned poses")
    synced_name: bpy.props.StringProperty(options={"HIDDEN"})


class MeatbagPoseResult(bpy.types.PropertyGroup):
    index: bpy.props.IntProperty()
    before: bpy.props.FloatProperty(subtype="DISTANCE", unit="LENGTH")
    after: bpy.props.FloatProperty(subtype="DISTANCE", unit="LENGTH")


def _blend_mode_changed(self, context):
    if self.armature and self.armature.meatbag_library:
        show_preview(context, self.armature.meatbag_library)


class MeatbagSettings(bpy.types.PropertyGroup):
    armature: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_armature, name="Armature",
                                        description="Armature whose pose library is edited and solved")
    mesh: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_mesh, name="Mesh",
                                    description="The skinned mesh whose helper bones are solved")
    target: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_mesh, name="Target",
                                      description="Mesh to match in each pose, with the same vertices as Mesh, such as a muscle skin. "
                                                  "Its shape keys named after a pose are on only for that pose. Empty uses the sculpt target")
    solve_scope: bpy.props.EnumProperty(name="Solve", items=(
        ("ALL", "All Poses", "Solve every pose that isn't locked"),
        ("ACTIVE", "Active Pose", "Solve only the active pose"),
    ))
    selected_helpers: bpy.props.BoolProperty(name="Selected Helpers Only",
                                             description="Solve only the selected helper bones; the others keep their pose")
    solve_on_back: bpy.props.BoolProperty(name="Solve on Back", default=True,
                                          description="Solve the sculpted pose when leaving the sculpt target")
    helper_collection: bpy.props.StringProperty(name="Helper Bones", default="Correctives",
                                                description="Bone collection of the helper bones to solve; empty uses the selected bones")
    allow_scale: bpy.props.BoolProperty(name="Allow Scale", default=False,
                                        description="Let helpers scale uniformly, for bulging muscles")
    solve_weights: bpy.props.BoolProperty(name="Solve Weights", default=False,
                                          description="Also refine how much each vertex follows its helpers; main bone weights keep their proportions")
    iterations: bpy.props.IntProperty(name="Iterations", default=5, min=1, max=100)
    weight_rounds: bpy.props.IntProperty(name="Weight Rounds", default=10, min=1, max=100)
    residual_keys: bpy.props.BoolProperty(name="Residual Shape Keys", default=True,
                                          description="Write what the bones can't reach as CR_<pose> shape keys")
    residual_threshold: bpy.props.FloatProperty(name="Threshold", default=0.05, min=0.0, subtype="DISTANCE", unit="LENGTH",
                                                description="Poses whose largest remaining error is below this get no residual shape key")
    blend_mode: bpy.props.EnumProperty(name="Blend", update=_blend_mode_changed, items=(
        (library.BLEND, "Blend", "Weighted average; the rest pose takes what is left below 1, so A at 1 - t and B at t interpolate between them"),
        (library.ADDITIVE, "Additive", "Each pose's change from rest is added on top of the others, like stacked correctives"),
    ))
    pose_asset: bpy.props.StringProperty(name="Pose Asset",
                                         description="Unreal Pose Asset the poses go to, such as /Game/Characters/Body/PA_Body_Correctives; created when missing")
    keep_source: bpy.props.BoolProperty(name="Keep Source Animation", default=False,
                                        description="Keep the imported animation (<Pose Asset>_Source) in Unreal instead of deleting it")
    results: bpy.props.CollectionProperty(type=MeatbagPoseResult)
    result_index: bpy.props.IntProperty()


def _rms(errors):
    return float(np.sqrt(np.mean(errors ** 2))) if errors.size else 0.0


# Pose library

class _LibraryOperator:
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return settings(context).armature is not None

    def library(self, context, create=False):
        armature = settings(context).armature
        return armature, library.library_of(armature, create)

    def finish(self, context, action, names, values, active):
        armature = settings(context).armature
        bones = library.bone_names(armature)
        library.write(action, names, bones, values)
        action.meatbag_pose_index = max(0, min(active, len(names) - 1))
        context.view_layer.update()
        return {"FINISHED"}


class MEATBAG_OT_pose_add(_LibraryOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_add"
    bl_label = "Add Pose"
    bl_description = "Add the viewport pose to the library, after the active pose"

    name: bpy.props.StringProperty(name="Name", default="pose")
    rest: bpy.props.BoolProperty(name="Rest Pose", description="Add the rest pose (all bone transforms cleared) instead of the viewport pose")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        armature, action = self.library(context, create=True)
        bones = library.bone_names(armature)
        names = library.pose_names(action)
        values = library.read(action, bones)
        pose = np.tile(library.REST, (len(bones), 1)) if self.rest else library.current(armature, bones)
        at = action.meatbag_pose_index + 1 if names else 0
        name = library.unique_name(action, self.name.strip() or "pose")
        names.insert(at, name)
        values = np.insert(values, at, pose, axis=0)
        if self.rest:
            library.set_pose(armature, bones, pose)
        return self.finish(context, action, names, values, at)


class _PoseOperator(_LibraryOperator):
    @classmethod
    def poll(cls, context):
        armature = settings(context).armature
        return bool(armature and armature.meatbag_library and armature.meatbag_library.meatbag_poses)


class MEATBAG_OT_pose_update(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_update"
    bl_label = "Update Pose"
    bl_description = "Replace the active pose with the viewport pose"

    def execute(self, context):
        armature, action = self.library(context)
        bones = library.bone_names(armature)
        values = library.read(action, bones)
        values[action.meatbag_pose_index] = library.current(armature, bones)
        return self.finish(context, action, library.pose_names(action), values, action.meatbag_pose_index)


class MEATBAG_OT_pose_remove(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_remove"
    bl_label = "Remove Pose"
    bl_description = "Remove the active pose from the library"

    def execute(self, context):
        armature, action = self.library(context)
        bones = library.bone_names(armature)
        at = action.meatbag_pose_index
        names = library.pose_names(action)
        values = np.delete(library.read(action, bones), at, axis=0)
        target = _sculpt_target(context)
        if target:
            sculpt.remove_pose_key(target, names[at])
        del names[at]
        return self.finish(context, action, names, values, at - 1 if at else 0)


class MEATBAG_OT_pose_move(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_move"
    bl_label = "Move Pose"
    bl_description = "Move the active pose up or down; the Pose Asset gets the poses in this order"

    direction: bpy.props.EnumProperty(items=(("UP", "Up", ""), ("DOWN", "Down", "")))

    def execute(self, context):
        armature, action = self.library(context)
        bones = library.bone_names(armature)
        at = action.meatbag_pose_index
        to = at - 1 if self.direction == "UP" else at + 1
        names = library.pose_names(action)
        if not 0 <= to < len(names):
            return {"CANCELLED"}
        values = library.read(action, bones)
        names[at], names[to] = names[to], names[at]
        values[[at, to]] = values[[to, at]]
        return self.finish(context, action, names, values, to)


class MEATBAG_OT_pose_apply(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_apply"
    bl_label = "Show Pose"
    bl_description = "Show this pose in the viewport (clears the preview weights)"

    index: bpy.props.IntProperty()

    def execute(self, context):
        armature, action = self.library(context)
        bones = library.bone_names(armature)
        values = library.read(action, bones)
        if not 0 <= self.index < len(values):
            return {"CANCELLED"}
        for pose in action.meatbag_poses:
            pose["weight"] = 0.0
        action.meatbag_pose_index = self.index
        library.set_pose(armature, bones, values[self.index])
        return {"FINISHED"}


class MEATBAG_OT_pose_rest(_LibraryOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_rest"
    bl_label = "Rest Pose"
    bl_description = "Clear every bone transform and the preview weights"

    def execute(self, context):
        armature, action = self.library(context)
        for pose in action.meatbag_poses if action else []:
            pose["weight"] = 0.0
        for pose_bone in armature.pose.bones:
            pose_bone.matrix_basis = Matrix.Identity(4)
        return {"FINISHED"}


def _helpers(s):
    helpers = capture.helper_bone_names(s.armature, s.helper_collection)
    if s.selected_helpers:
        selected = set(capture.helper_bone_names(s.armature, ""))
        helpers = [name for name in helpers if name in selected]
    return helpers


def _pose_indices(s, scope):
    action = library.library_of(s.armature)
    poses = action.meatbag_poses if action else []
    if scope == "ACTIVE":
        return [action.meatbag_pose_index] if poses else []
    return [i for i, pose in enumerate(poses) if not pose.locked]


def solve(operator, context, scope):
    """Captures, solves and writes back the scoped poses; reports to the operator."""
    s = settings(context)
    target = sculpt.effective_target(s)
    if target is None or target == s.mesh:
        operator.report({"ERROR"}, "No target: sculpt a pose, or pick a Target mesh")
        return {"CANCELLED"}
    helpers = _helpers(s)
    indices = _pose_indices(s, scope)
    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    try:
        captured = capture.capture(context, s.armature, s.mesh, target, helpers, indices)
    except capture.CaptureError as error:
        operator.report({"ERROR"}, str(error))
        return {"CANCELLED"}

    all_poses = len(library.pose_names(library.library_of(s.armature)))
    solve_weights = s.solve_weights and len(indices) == all_poses
    problem = captured.problem
    unweighted = [name for name in helpers if not (problem.weights[:, captured.bones.index(name)] > 0).any()]
    options = solver.Options(iterations=s.iterations, allow_scale=s.allow_scale, solve_weights=solve_weights,
                             weight_rounds=s.weight_rounds, max_influences=preferences.get(context).max_influences)
    result = solver.solve(problem, options)

    apply.write_helper_poses(context, s.armature, captured, result)
    changed = apply.write_weights(s.mesh, captured, result.weights) if solve_weights else 0
    apply.write_residual(s.mesh, result)
    keys = apply.write_residual_shape_keys(s.mesh, captured, result, s.residual_threshold) if s.residual_keys else []

    s.results.clear()
    for row_index, (name, index) in enumerate(zip(captured.poses, captured.indices)):
        row = s.results.add()
        row.name, row.index = name, index
        row.before = _rms(result.initial_error[row_index])
        row.after = _rms(result.error[row_index])

    message = f"Solved {len(helpers)} helpers over {len(captured.poses)} poses: RMS {result.rms(result.initial_error):.4g} to {result.rms():.4g}"
    if changed:
        message += f", {changed} vertices reweighted"
    if keys:
        message += f", {len(keys)} residual shape keys"
    if s.solve_weights and not solve_weights:
        message += ". Weights not solved: they need every pose, unlocked"
    if unweighted:
        operator.report({"WARNING"}, message + f". No weights, so not solved: {', '.join(unweighted)}")
    else:
        operator.report({"INFO"}, message)
    return {"FINISHED"}


class MEATBAG_OT_solve_correctives(bpy.types.Operator):
    bl_idname = "meatbag.solve_correctives"
    bl_label = "Solve Helper Bones"
    bl_description = ("Fit the helper bones so the skinned mesh matches the target in the poses chosen by Solve. "
                      "Stores the helpers in the poses and the remaining error in the mf_residual attribute")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.armature and s.mesh and s.armature.meatbag_library)

    def execute(self, context):
        return solve(self, context, settings(context).solve_scope)


class MEATBAG_OT_sculpt_pose(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.sculpt_pose"
    bl_label = "Sculpt Pose"
    bl_description = ("Show the active pose on the sculpt target (a hidden copy of the body, made when missing) "
                      "and sculpt its shape key for this pose. The body is hidden until Back")

    @classmethod
    def poll(cls, context):
        return super().poll(context) and settings(context).mesh is not None

    def execute(self, context):
        s = settings(context)
        armature, action = self.library(context)
        try:
            target = sculpt.ensure_target(s.mesh)
        except sculpt.SculptError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        index = action.meatbag_pose_index
        name = action.meatbag_poses[index].name
        key = sculpt.pose_key(target, name, create=True)

        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        bones = library.bone_names(armature)
        for pose in action.meatbag_poses:
            pose["weight"] = 0.0
        library.set_pose(armature, bones, library.read(action, bones)[index])

        target.active_shape_key_index = list(target.data.shape_keys.key_blocks).index(key)
        target.show_only_shape_key = True
        target.hide_set(False)
        s.mesh.hide_set(True)
        for obj in context.selected_objects:
            obj.select_set(False)
        target.select_set(True)
        context.view_layer.objects.active = target
        bpy.ops.object.mode_set(mode="SCULPT")
        self.report({"INFO"}, f"Sculpting {name}. Press Back when done")
        return {"FINISHED"}


class MEATBAG_OT_sculpt_back(bpy.types.Operator):
    bl_idname = "meatbag.sculpt_back"
    bl_label = "Back"
    bl_description = "Hide the sculpt target, show the body again, and solve the sculpted pose when Solve on Back is on"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.mesh and s.mesh.meatbag_target)

    def execute(self, context):
        s = settings(context)
        target = s.mesh.meatbag_target
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        target.show_only_shape_key = False
        target.hide_set(True)
        s.mesh.hide_set(False)
        for obj in context.selected_objects:
            obj.select_set(False)
        s.mesh.select_set(True)
        context.view_layer.objects.active = s.mesh
        if s.solve_on_back and s.armature and not s.target:
            return solve(self, context, "ACTIVE")
        return {"FINISHED"}


class MEATBAG_OT_send_poses(bpy.types.Operator):
    bl_idname = "meatbag.send_poses"
    bl_label = "Send Poses to Unreal"
    bl_description = ("Send every library pose (main and helper bones) to the Pose Asset: poses with the same name "
                      "are updated, the others created. Push the mesh first so Unreal has the helper bones")

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.armature and s.mesh and s.pose_asset)

    def execute(self, context):
        s = settings(context)
        if not s.mesh.fv_ue_asset or not s.mesh.fv_ue_skeleton:
            self.report({"ERROR"}, f"{s.mesh.name} isn't linked to an Unreal skeletal mesh; link it in the Unreal panel and push it first")
            return {"CANCELLED"}
        if not s.pose_asset.startswith("/"):
            self.report({"ERROR"}, "The Pose Asset path must start with /Game or a plugin mount")
            return {"CANCELLED"}
        poses = library.pose_names(library.library_of(s.armature))
        try:
            capture.check_names(poses)
        except capture.CaptureError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")

        filepath = export.temp_fbx_path(s.pose_asset + "_Source")
        pose_asset.export_poses(context, s.armature, filepath)
        scene = context.scene
        params = {
            "fbx": filepath,
            "pose_asset": s.pose_asset,
            "mesh": s.mesh.fv_ue_asset,
            "skeleton": s.mesh.fv_ue_skeleton,
            "poses": poses,
            "frame_rate": scene.render.fps / scene.render.fps_base,
            "keep_source": s.keep_source,
        }
        try:
            sent, _ = link.call("send_poses", params, preferences.remote_config(context), preferences.project_dir(context))
        except link.LinkError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}

        made = "Created" if sent["created_asset"] else "Updated"
        self.report({"INFO"}, f"{made} {sent['pose_asset']}: {len(sent['created'])} poses created, "
                              f"{len(sent['updated'])} updated. Save in Unreal to keep it.")
        return {"FINISHED"}


class MEATBAG_OT_use_selected_pose_asset(bpy.types.Operator):
    bl_idname = "meatbag.use_selected_pose_asset"
    bl_label = "Use Selected Pose Asset"
    bl_description = "Use the Pose Asset selected in Unreal's Content Browser"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        try:
            asset, _ = link.call("selected_asset", None, preferences.remote_config(context), preferences.project_dir(context))
        except link.LinkError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        if asset["class"] != "PoseAsset":
            self.report({"ERROR"}, f"{asset['path']} is a {asset['class']}, not a Pose Asset")
            return {"CANCELLED"}
        settings(context).pose_asset = asset["path"]
        return {"FINISHED"}


class MEATBAG_UL_poses(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index):
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_pose_apply.bl_idname, text="", icon="ARMATURE_DATA", emboss=False).index = index
        row.prop(item, "name", text="", emboss=False)
        row.prop(item, "weight", text="", slider=True)
        row.prop(item, "locked", text="", icon="LOCKED" if item.locked else "UNLOCKED", emboss=False)


class MEATBAG_UL_pose_results(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index):
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_pose_apply.bl_idname, text="", icon="ARMATURE_DATA", emboss=False).index = item.index
        row.label(text=item.name)
        row.prop(item, "before", text="", emboss=False)
        row.prop(item, "after", text="", emboss=False)


classes = (
    MeatbagPose,
    MeatbagPoseResult,
    MeatbagSettings,
    MEATBAG_OT_pose_add,
    MEATBAG_OT_pose_update,
    MEATBAG_OT_pose_remove,
    MEATBAG_OT_pose_move,
    MEATBAG_OT_pose_apply,
    MEATBAG_OT_pose_rest,
    MEATBAG_OT_solve_correctives,
    MEATBAG_OT_sculpt_pose,
    MEATBAG_OT_sculpt_back,
    MEATBAG_OT_send_poses,
    MEATBAG_OT_use_selected_pose_asset,
    MEATBAG_UL_poses,
    MEATBAG_UL_pose_results,
)


def register_properties():
    bpy.types.Scene.meatbag = bpy.props.PointerProperty(type=MeatbagSettings)
    bpy.types.Object.meatbag_library = bpy.props.PointerProperty(type=bpy.types.Action, name="Pose Library",
                                                                 description="Action holding this armature's Meatbag Factory poses, pose i at frame i")
    bpy.types.Action.meatbag_poses = bpy.props.CollectionProperty(type=MeatbagPose)
    bpy.types.Action.meatbag_pose_index = bpy.props.IntProperty()
    bpy.types.Object.meatbag_target = bpy.props.PointerProperty(type=bpy.types.Object, name="Sculpt Target",
                                                                description="Hidden copy of this body that holds the per-pose sculpts")


def unregister_properties():
    del bpy.types.Action.meatbag_pose_index
    del bpy.types.Action.meatbag_poses
    del bpy.types.Object.meatbag_target
    del bpy.types.Object.meatbag_library
    del bpy.types.Scene.meatbag
