"""Meatbag Factory correctives: the pose library, the helper bone solve, and sending the poses to a Pose Asset in Unreal."""

import numpy as np

import bpy
from mathutils import Matrix

from . import apply, capture, library, pose_asset, solver
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


_renaming = set()


def _name_changed(pose, context):
    key = pose.as_pointer()
    if key in _renaming:
        return
    others = [other.name for other in pose.id_data.meatbag_poses if other.as_pointer() != key]
    if pose.name in others or not pose.name:
        base = pose.name or "pose"
        name, suffix = base, 1
        while name in others:
            name, suffix = f"{base}_{suffix}", suffix + 1
        _renaming.add(key)
        try:
            pose.name = name
        finally:
            _renaming.discard(key)


class MeatbagPose(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="Name", update=_name_changed,
                                   description="Pose name in Unreal, with its side token (thigh_l_raise_90)")
    weight: bpy.props.FloatProperty(name="Weight", default=0.0, soft_min=0.0, soft_max=1.0, update=_weight_changed,
                                    description="Preview weight; the viewport shows the mix of all weights")


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
                                      description="The mesh to match in each pose, with the same vertices as Mesh. Its shape keys named after a pose are on only for that pose")
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


class MEATBAG_OT_solve_correctives(bpy.types.Operator):
    bl_idname = "meatbag.solve_correctives"
    bl_label = "Solve Helper Bones"
    bl_description = ("Fit the helper bones so the skinned mesh matches the target in every library pose. "
                      "Stores the helpers in the poses and the remaining error in the mf_residual attribute")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.armature and s.mesh and s.target and s.mesh != s.target)

    def execute(self, context):
        s = settings(context)
        helpers = capture.helper_bone_names(s.armature, s.helper_collection)
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        try:
            captured = capture.capture(context, s.armature, s.mesh, s.target, helpers)
        except capture.CaptureError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}

        problem = captured.problem
        unweighted = [name for name in helpers if not (problem.weights[:, captured.bones.index(name)] > 0).any()]
        options = solver.Options(iterations=s.iterations, allow_scale=s.allow_scale, solve_weights=s.solve_weights,
                                 weight_rounds=s.weight_rounds, max_influences=preferences.get(context).max_influences)
        result = solver.solve(problem, options)

        apply.write_helper_poses(context, s.armature, captured, result)
        changed = apply.write_weights(s.mesh, captured, result.weights) if s.solve_weights else 0
        apply.write_residual(s.mesh, result)
        keys = apply.write_residual_shape_keys(s.mesh, captured, result, s.residual_threshold) if s.residual_keys else []

        s.results.clear()
        for index, name in enumerate(captured.poses):
            row = s.results.add()
            row.name, row.index = name, index
            row.before = _rms(result.initial_error[index])
            row.after = _rms(result.error[index])

        message = f"Solved {len(helpers)} helpers over {len(captured.poses)} poses: RMS {result.rms(result.initial_error):.4g} to {result.rms():.4g}"
        if changed:
            message += f", {changed} vertices reweighted"
        if keys:
            message += f", {len(keys)} residual shape keys"
        if unweighted:
            self.report({"WARNING"}, message + f". No weights, so not solved: {', '.join(unweighted)}")
        else:
            self.report({"INFO"}, message)
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


def unregister_properties():
    del bpy.types.Action.meatbag_pose_index
    del bpy.types.Action.meatbag_poses
    del bpy.types.Object.meatbag_library
    del bpy.types.Scene.meatbag
