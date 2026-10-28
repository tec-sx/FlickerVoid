"""Meatbag Factory correctives: solve helper bones against a target mesh, and send the poses to a Pose Asset in Unreal."""

import numpy as np

import bpy

from . import apply, capture, pose_asset, solver
from .. import preferences
from ..push import export
from ..unreal_link import link


def _is_armature(_, obj):
    return obj.type == "ARMATURE"


def _is_mesh(_, obj):
    return obj.type == "MESH"


class MeatbagPoseResult(bpy.types.PropertyGroup):
    frame: bpy.props.IntProperty()
    before: bpy.props.FloatProperty(subtype="DISTANCE", unit="LENGTH")
    after: bpy.props.FloatProperty(subtype="DISTANCE", unit="LENGTH")


class MeatbagSettings(bpy.types.PropertyGroup):
    armature: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_armature, name="Armature")
    mesh: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_mesh, name="Mesh",
                                    description="The skinned mesh whose helper bones are solved")
    target: bpy.props.PointerProperty(type=bpy.types.Object, poll=_is_mesh, name="Target",
                                      description="The mesh to match in each pose (muscle simulation, sculpt), with the same vertices as Mesh")
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
    pose_asset: bpy.props.StringProperty(name="Pose Asset",
                                         description="Unreal Pose Asset the poses go to, such as /Game/Characters/Body/PA_Body_Correctives; created when missing")
    keep_source: bpy.props.BoolProperty(name="Keep Source Animation", default=False,
                                        description="Keep the imported animation (<Pose Asset>_Source) in Unreal instead of deleting it")
    results: bpy.props.CollectionProperty(type=MeatbagPoseResult)
    result_index: bpy.props.IntProperty()


def settings(context):
    return context.scene.meatbag


def _rms(errors):
    return float(np.sqrt(np.mean(errors ** 2))) if errors.size else 0.0


class MEATBAG_OT_solve_correctives(bpy.types.Operator):
    bl_idname = "meatbag.solve_correctives"
    bl_label = "Solve Helper Bones"
    bl_description = ("Fit the helper bones so the skinned mesh matches the target in every pose marker. "
                      "Keys the helpers at each marker and stores the remaining error in the mf_residual attribute")
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

        apply.write_helper_keys(context, s.armature, captured, result)
        changed = apply.write_weights(s.mesh, captured, result.weights) if s.solve_weights else 0
        apply.write_residual(s.mesh, result)
        keys = apply.write_residual_shape_keys(s.mesh, captured, result, s.residual_threshold) if s.residual_keys else []

        s.results.clear()
        for index, (name, frame) in enumerate(captured.poses):
            row = s.results.add()
            row.name, row.frame = name, frame
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
    bl_description = ("Send every pose marker's pose (main and helper bones) to the Pose Asset: poses with the "
                      "marker's name are updated, the others created. Push the mesh first so Unreal has the helper bones")

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
        try:
            poses = capture.pose_markers(context.scene)
        except capture.CaptureError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        if not poses:
            self.report({"ERROR"}, "Add a timeline marker for each pose, named after the pose")
            return {"CANCELLED"}
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")

        filepath = export.temp_fbx_path(s.pose_asset + "_Source")
        pose_asset.export_poses(context, s.armature, poses, filepath)
        scene = context.scene
        params = {
            "fbx": filepath,
            "pose_asset": s.pose_asset,
            "mesh": s.mesh.fv_ue_asset,
            "skeleton": s.mesh.fv_ue_skeleton,
            "poses": [name for name, _ in poses],
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


class MEATBAG_OT_go_to_pose(bpy.types.Operator):
    bl_idname = "meatbag.go_to_pose"
    bl_label = "Go To Pose"
    bl_description = "Jump to this pose's marker"

    frame: bpy.props.IntProperty()

    def execute(self, context):
        context.scene.frame_set(self.frame)
        return {"FINISHED"}


class MEATBAG_UL_pose_results(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index):
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_go_to_pose.bl_idname, text="", icon="MARKER_HLT", emboss=False).frame = item.frame
        row.label(text=item.name)
        row.prop(item, "before", text="", emboss=False)
        row.prop(item, "after", text="", emboss=False)


classes = (
    MeatbagPoseResult,
    MeatbagSettings,
    MEATBAG_OT_solve_correctives,
    MEATBAG_OT_send_poses,
    MEATBAG_OT_use_selected_pose_asset,
    MEATBAG_OT_go_to_pose,
    MEATBAG_UL_pose_results,
)


def register_properties():
    bpy.types.Scene.meatbag = bpy.props.PointerProperty(type=MeatbagSettings)


def unregister_properties():
    del bpy.types.Scene.meatbag
