"""Meatbag Factory correctives: the pose library, the helper bone solve, and sending the poses to a Pose Asset in Unreal."""

import numpy as np

import bpy
from mathutils import Matrix

from . import apply, capture, library, pose_asset, sculpt, solver
from .. import preferences
from ..push import export
from ..unreal_link import link


HIDDEN_MIRRORS = "mf_hidden_mirrors"


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


def preview_weights(action):
    return [pose.weight for pose in action.meatbag_poses]


def show_preview(context, action):
    """Poses the library's armature as the mix of the poses' preview weights, and sets each pose's blend shape on the body to its weight."""
    armature = _library_armature(context, action)
    if armature is None:
        return
    bones = library.bone_names(armature)
    weights = preview_weights(action)
    library.set_pose(armature, bones, library.mix(library.read(action, bones), weights, settings(context).blend_mode))
    mesh = settings(context).mesh
    if mesh is None:
        return
    for pose, weight in zip(action.meatbag_poses, weights):
        key = sculpt.pose_key(mesh, sculpt.shape_name(pose.name))
        if key is not None:
            key.value = weight


def show_only(context, action, index):
    """Shows one pose at full weight; the others go to 0."""
    for i, pose in enumerate(action.meatbag_poses):
        pose["weight"] = 1.0 if i == index else 0.0
    action.meatbag_pose_index = index
    show_preview(context, action)


def _weight_changed(pose, context):
    show_preview(context, pose.id_data)


def _name_changed(pose, context):
    """Keeps names unique and renames the pose's blend shape and sculpt target shape key with it."""
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
    mesh = settings(context).mesh
    if old and mesh and old != name:
        sculpt.rename_pose_key(mesh, sculpt.shape_name(old), sculpt.shape_name(name))
    pose["synced_name"] = name


def _sculpt_target(context):
    mesh = settings(context).mesh
    return mesh.meatbag_target if mesh else None


class MeatbagBone(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="Bone")


class MeatbagPose(bpy.types.PropertyGroup):
    name: bpy.props.StringProperty(name="Name", update=_name_changed,
                                   description="Pose name in Unreal, with its side token (thigh_l_raise_90)")
    weight: bpy.props.FloatProperty(name="Weight", default=0.0, soft_min=0.0, soft_max=1.0, update=_weight_changed,
                                    description="Preview weight; the viewport shows the mix of all weights")
    locked: bpy.props.BoolProperty(name="Locked", description="Solve leaves this pose's helpers as they are, for hand-tuned poses")
    control_bone: bpy.props.StringProperty(name="Control Bone",
                                           description="The bone whose rotation drives this pose in Unreal, like a pose reader's driver (upperarm_l for upperarm_l_up)")
    affected_bones: bpy.props.CollectionProperty(type=MeatbagBone, name="Affected Bones",
                                                 description="The bones this pose moves; Update Pose changes only these, and Send Poses sends only these and the control bone")
    affected_index: bpy.props.IntProperty()
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
    affected_only: bpy.props.BoolProperty(name="Affected Bones Only", default=True,
                                          description="Send only each pose's control and affected bones; the others go at rest. Poses without affected bones send every bone")
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


def _selection(armature):
    """The active bone and the other selected bones of the armature."""
    active = armature.data.bones.active
    active_name = active.name if active else ""
    selected = [bone.name for bone in armature.data.bones if capture.is_selected(armature, bone) and bone.name != active_name]
    return active_name, selected


def _set_bones(pose, control, affected):
    pose.control_bone = control
    pose.affected_bones.clear()
    for name in affected:
        if name != control:
            pose.affected_bones.add().name = name


class MEATBAG_OT_pose_add(_LibraryOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_add"
    bl_label = "Add Pose"
    bl_description = "Add the viewport pose to the library, after the active pose"

    name: bpy.props.StringProperty(name="Name", default="pose")
    rest: bpy.props.BoolProperty(name="Rest Pose", description="Add the rest pose (all bone transforms cleared) instead of the viewport pose")
    from_selection: bpy.props.BoolProperty(name="Bones from Selection", default=True,
                                           description="The active bone becomes the control bone, the other selected bones the affected bones")

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
        result = self.finish(context, action, names, values, at)
        if self.from_selection:
            control, affected = _selection(armature)
            _set_bones(action.meatbag_poses[at], control, affected)
        show_only(context, action, at)
        return result


class _PoseOperator(_LibraryOperator):
    @classmethod
    def poll(cls, context):
        armature = settings(context).armature
        return bool(armature and armature.meatbag_library and armature.meatbag_library.meatbag_poses)

    def active_pose(self, context):
        action = settings(context).armature.meatbag_library
        return action.meatbag_poses[action.meatbag_pose_index]


def update_pose(operator, context, control):
    """Adds the viewport change since the library last posed the armature onto the active pose's control or affected bones."""
    s = settings(context)
    armature = s.armature
    action = armature.meatbag_library
    bones = library.bone_names(armature)
    index = {name: i for i, name in enumerate(bones)}
    at = action.meatbag_pose_index
    pose = action.meatbag_poses[at]
    if control:
        names = [pose.control_bone] if pose.control_bone in index else []
        if not names:
            operator.report({"ERROR"}, f"{pose.name} has no control bone; set one first")
            return {"CANCELLED"}
    else:
        names = [bone.name for bone in pose.affected_bones if bone.name in index]
        if pose.affected_bones and not names:
            operator.report({"ERROR"}, f"None of {pose.name}'s affected bones are in {armature.name}")
            return {"CANCELLED"}
        names = names or bones

    values = library.read(action, bones)
    weights = preview_weights(action)
    shown = library.mix(values, weights, s.blend_mode)
    viewport = library.current(armature, bones)
    rows = [index[name] for name in names]
    values[at] = library.add_change(values[at], shown, viewport, rows)
    library.write(action, library.pose_names(action), bones, values)

    # The updated bones now show the new mix; edits to other bones stay in the viewport.
    mixed = library.mix(values, weights, s.blend_mode)
    library.set_pose(armature, names, mixed[rows])
    context.view_layer.update()

    updated = set(rows)
    skipped = [bones[row] for row in library.changed_rows(shown, viewport) if row not in updated]
    if skipped:
        listed = ", ".join(skipped[:5]) + (f" and {len(skipped) - 5} more" if len(skipped) > 5 else "")
        operator.report({"WARNING"}, f"Updated {pose.name}. Moved but not saved, since they aren't its {'control' if control else 'affected'} bones: {listed}")
    else:
        operator.report({"INFO"}, f"Updated {pose.name}: {len(names)} bones")
    return {"FINISHED"}


class MEATBAG_OT_pose_update(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_update"
    bl_label = "Update Pose"
    bl_description = ("Add your viewport changes to the active pose's affected bones (every bone when it has none). "
                      "Only the change since the library last posed the armature is added, so this works while poses are blended")

    def execute(self, context):
        return update_pose(self, context, control=False)


class MEATBAG_OT_pose_update_control(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_update_control"
    bl_label = "Update Control"
    bl_description = "Add your viewport change of the control bone to the active pose, the same way as Update Pose"

    def execute(self, context):
        return update_pose(self, context, control=True)


class MEATBAG_OT_pose_bones_from_selection(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_bones_from_selection"
    bl_label = "Bones from Selection"
    bl_description = "The active bone becomes the active pose's control bone, the other selected bones its affected bones"

    def execute(self, context):
        control, affected = _selection(settings(context).armature)
        _set_bones(self.active_pose(context), control, affected)
        return {"FINISHED"}


class MEATBAG_OT_pose_affected_add(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_affected_add"
    bl_label = "Add Selected Bones"
    bl_description = "Add the selected bones to the active pose's affected bones"

    def execute(self, context):
        pose = self.active_pose(context)
        armature = settings(context).armature
        have = {bone.name for bone in pose.affected_bones}
        for bone in armature.data.bones:
            if capture.is_selected(armature, bone) and bone.name not in have and bone.name != pose.control_bone:
                pose.affected_bones.add().name = bone.name
        return {"FINISHED"}


class MEATBAG_OT_pose_affected_remove(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_affected_remove"
    bl_label = "Remove Bone"
    bl_description = "Remove the highlighted bone from the active pose's affected bones; its stored transform stays until you send or update"

    def execute(self, context):
        pose = self.active_pose(context)
        if not pose.affected_bones:
            return {"CANCELLED"}
        pose.affected_bones.remove(pose.affected_index)
        pose.affected_index = max(0, min(pose.affected_index, len(pose.affected_bones) - 1))
        return {"FINISHED"}


class MEATBAG_OT_pose_remove(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_remove"
    bl_label = "Remove Pose"
    bl_description = "Remove the active pose from the library, with its blend shape and sculpt target shape key"

    def execute(self, context):
        armature, action = self.library(context)
        bones = library.bone_names(armature)
        at = action.meatbag_pose_index
        names = library.pose_names(action)
        values = np.delete(library.read(action, bones), at, axis=0)
        target = _sculpt_target(context)
        if target:
            sculpt.remove_pose_key(target, names[at])
        mesh = settings(context).mesh
        if mesh:
            sculpt.remove_pose_key(mesh, sculpt.shape_name(names[at]))
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
    bl_description = "Show only this pose, at full weight, with its blend shape"

    index: bpy.props.IntProperty()

    def execute(self, context):
        _, action = self.library(context)
        if not 0 <= self.index < len(action.meatbag_poses):
            return {"CANCELLED"}
        show_only(context, action, self.index)
        return {"FINISHED"}


class MEATBAG_OT_pose_rest(_LibraryOperator, bpy.types.Operator):
    bl_idname = "meatbag.pose_rest"
    bl_label = "Rest Pose"
    bl_description = "Clear every bone transform, the preview weights and the pose blend shapes"

    def execute(self, context):
        armature, action = self.library(context)
        for pose in action.meatbag_poses if action else []:
            pose["weight"] = 0.0
        for pose_bone in armature.pose.bones:
            pose_bone.matrix_basis = Matrix.Identity(4)
        if action:
            show_preview(context, action)
        return {"FINISHED"}


def _helpers(s, pose):
    """The pose's deform affected bones, else the helper collection (or selection); limited to the selection with Selected Helpers Only."""
    bones = s.armature.data.bones
    helpers = [bone.name for bone in pose.affected_bones if bone.name in bones and bones[bone.name].use_deform]
    if not pose.affected_bones:
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


def _groups(s, indices):
    """Poses that solve the same helpers, as {helper names: [pose indices]}, in library order."""
    poses = library.library_of(s.armature).meatbag_poses
    groups = {}
    for index in indices:
        groups.setdefault(tuple(_helpers(s, poses[index])), []).append(index)
    return groups


def solve(operator, context, scope):
    """Captures, solves and writes back the scoped poses, each pose on its own affected bones; reports to the operator."""
    s = settings(context)
    target = s.mesh.meatbag_target
    if target is None:
        operator.report({"ERROR"}, "No sculpt target yet: press Sculpt Target first")
        return {"CANCELLED"}
    if sculpt.is_stale(s.mesh, target):
        operator.report({"ERROR"}, sculpt.stale_message(s.mesh, target))
        return {"CANCELLED"}
    indices = _pose_indices(s, scope)
    groups = _groups(s, indices)
    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")

    all_poses = len(library.pose_names(library.library_of(s.armature)))
    solve_weights = s.solve_weights and len(indices) == all_poses and len(groups) == 1
    options = solver.Options(iterations=s.iterations, allow_scale=s.allow_scale, solve_weights=solve_weights,
                             weight_rounds=s.weight_rounds, max_influences=preferences.get(context).max_influences)
    s.results.clear()
    solved, unweighted, keys = set(), set(), []
    initial_errors, final_errors, changed = [], [], 0
    for helpers, group in groups.items():
        try:
            captured = capture.capture(context, s.armature, s.mesh, target, list(helpers), group)
        except capture.CaptureError as error:
            operator.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        problem = captured.problem
        unweighted.update(name for name in helpers if not (problem.weights[:, captured.bones.index(name)] > 0).any())
        result = solver.solve(problem, options)

        apply.write_helper_poses(context, s.armature, captured, result)
        if solve_weights:
            changed = apply.write_weights(s.mesh, captured, result.weights)
        if s.residual_keys:
            keys += apply.write_residual_shape_keys(s.mesh, captured, result, s.residual_threshold)
        initial_errors.append(result.initial_error)
        final_errors.append(result.error)
        solved.update(helpers)
        for row_index, (name, index) in enumerate(zip(captured.poses, captured.indices)):
            row = s.results.add()
            row.name, row.index = name, index
            row.before = _rms(result.initial_error[row_index])
            row.after = _rms(result.error[row_index])
    apply.write_residual(s.mesh, np.concatenate(final_errors))

    before = _rms(np.concatenate(initial_errors))
    after = _rms(np.concatenate(final_errors))
    message = f"Solved {len(solved)} helpers over {len(indices)} poses: RMS {before:.4g} to {after:.4g}"
    if changed:
        message += f", {changed} vertices reweighted"
    if keys:
        message += f", {len(keys)} residual shape keys"
    if s.solve_weights and not solve_weights:
        message += ". Weights not solved: they need every pose, unlocked, sharing the same helpers"
    if unweighted:
        operator.report({"WARNING"}, message + f". No weights, so not solved: {', '.join(sorted(unweighted))}")
    else:
        operator.report({"INFO"}, message)
    return {"FINISHED"}


class MEATBAG_OT_solve_correctives(bpy.types.Operator):
    bl_idname = "meatbag.solve_correctives"
    bl_label = "Solve Helper Bones"
    bl_description = ("Fit the helper bones so the skinned mesh matches the target in the poses chosen by Solve: each pose "
                      "its affected bones, or the helper collection when it has none. Stores the helpers in the poses and the "
                      "remaining error in the mf_residual attribute")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.armature and s.mesh and s.armature.meatbag_library)

    def execute(self, context):
        return solve(self, context, settings(context).solve_scope)


def _sculpt(context, obj):
    for other in context.selected_objects:
        other.select_set(False)
    obj.select_set(True)
    context.view_layer.objects.active = obj
    bpy.ops.object.mode_set(mode="SCULPT")


class MEATBAG_OT_sculpt_shape(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.sculpt_shape"
    bl_label = "Sculpt Shape"
    bl_description = ("Sculpt the active pose's blend shape (the body's CR_<pose> shape key) in the pose. "
                      "Unreal gets it as a morph target that the pose drives")

    @classmethod
    def poll(cls, context):
        return super().poll(context) and settings(context).mesh is not None

    def execute(self, context):
        s = settings(context)
        _, action = self.library(context)
        index = action.meatbag_pose_index
        name = action.meatbag_poses[index].name
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        key = sculpt.pose_key(s.mesh, sculpt.shape_name(name), create=True)
        show_only(context, action, index)
        s.mesh.show_only_shape_key = False
        s.mesh.active_shape_key_index = list(s.mesh.data.shape_keys.key_blocks).index(key)
        # A blend shape on the modelled half would show on both sides; the push puts it on its own side.
        hidden = [m.name for m in s.mesh.modifiers if m.type == "MIRROR" and m.show_viewport]
        for name in hidden:
            s.mesh.modifiers[name].show_viewport = False
        s.mesh[HIDDEN_MIRRORS] = hidden
        _sculpt(context, s.mesh)
        self.report({"INFO"}, f"Sculpting {key.name} in {name}. Press Back when done")
        return {"FINISHED"}


class MEATBAG_OT_sculpt_pose(_PoseOperator, bpy.types.Operator):
    bl_idname = "meatbag.sculpt_pose"
    bl_label = "Sculpt Target"
    bl_description = ("Show the active pose on the sculpt target (a hidden copy of the body, made when missing) "
                      "and sculpt the full shape wanted in this pose, for the solve. The body is hidden until Back")

    @classmethod
    def poll(cls, context):
        return super().poll(context) and settings(context).mesh is not None

    def execute(self, context):
        s = settings(context)
        _, action = self.library(context)
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
        show_only(context, action, index)

        target.active_shape_key_index = list(target.data.shape_keys.key_blocks).index(key)
        target.show_only_shape_key = True
        target.hide_set(False)
        s.mesh.hide_set(True)
        _sculpt(context, target)
        self.report({"INFO"}, f"Sculpting {name}. Press Back when done")
        return {"FINISHED"}


class MEATBAG_OT_rebuild_target(bpy.types.Operator):
    bl_idname = "meatbag.rebuild_target"
    bl_label = "Rebuild Target"
    bl_description = "Delete the sculpt target and make a new one from the body as it is now, with an empty shape key per pose. Its sculpts are lost"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = settings(context)
        return bool(s.mesh and s.mesh.meatbag_target and sculpting(context) is None)

    def execute(self, context):
        s = settings(context)
        sculpt.remove_target(s.mesh)
        target = sculpt.ensure_target(s.mesh)
        action = s.armature.meatbag_library if s.armature else None
        for name in library.pose_names(action):
            sculpt.pose_key(target, name, create=True)
        self.report({"INFO"}, f"Rebuilt {target.name} with {len(target.data.vertices)} vertices")
        return {"FINISHED"}


def sculpting(context):
    """What is being sculpted: "SHAPE" for a pose blend shape on the body, "TARGET" for the sculpt target, else None."""
    s = settings(context)
    active = context.active_object
    if s.mesh is None or active is None:
        return None
    if active == s.mesh.meatbag_target:
        return "TARGET"
    if active == s.mesh and context.mode == "SCULPT":
        return "SHAPE"
    return None


class MEATBAG_OT_sculpt_back(bpy.types.Operator):
    bl_idname = "meatbag.sculpt_back"
    bl_label = "Back"
    bl_description = ("Leave sculpting. From the sculpt target: show the body again, and solve the sculpted pose when "
                      "Solve on Back is on")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return sculpting(context) is not None

    def execute(self, context):
        s = settings(context)
        what = sculpting(context)
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        if what == "SHAPE":
            for name in s.mesh.get(HIDDEN_MIRRORS, []):
                modifier = s.mesh.modifiers.get(name)
                if modifier:
                    modifier.show_viewport = True
            if HIDDEN_MIRRORS in s.mesh:
                del s.mesh[HIDDEN_MIRRORS]
            if s.armature and s.armature.meatbag_library:
                show_preview(context, s.armature.meatbag_library)
            return {"FINISHED"}
        target = s.mesh.meatbag_target
        target.show_only_shape_key = False
        target.hide_set(True)
        s.mesh.hide_set(False)
        for obj in context.selected_objects:
            obj.select_set(False)
        s.mesh.select_set(True)
        context.view_layer.objects.active = s.mesh
        if s.solve_on_back and s.armature:
            return solve(self, context, "ACTIVE")
        return {"FINISHED"}


class MEATBAG_OT_send_poses(bpy.types.Operator):
    bl_idname = "meatbag.send_poses"
    bl_label = "Send Poses to Unreal"
    bl_description = ("Send every library pose to the Pose Asset: poses with the same name are updated, the others created. "
                      "A pose with a blend shape gets its CR_<pose> curve at 1. Push the mesh first so Unreal has the bones and morph targets")

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

        action = library.library_of(s.armature)
        keep = [_sent_bones(pose) if s.affected_only else None for pose in action.meatbag_poses]
        filepath = export.temp_fbx_path(s.pose_asset + "_Source")
        pose_asset.export_poses(context, s.armature, filepath, keep)
        curves = {pose: sculpt.shape_name(pose) for pose in poses if sculpt.pose_key(s.mesh, sculpt.shape_name(pose))}
        # Counterpart curves (CR_..._r), so Mirror All in Unreal copies each side curve onto them.
        sides = preferences.contract(context).sides
        counterparts = [name for name in (sides.mirror_name(curve) for curve in curves.values()) if name and name not in curves.values()]
        bones = {pose.name: {"control": pose.control_bone, "affected": [bone.name for bone in pose.affected_bones]}
                 for pose in action.meatbag_poses if pose.control_bone or pose.affected_bones}
        scene = context.scene
        params = {
            "fbx": filepath,
            "pose_asset": s.pose_asset,
            "mesh": s.mesh.fv_ue_asset,
            "skeleton": s.mesh.fv_ue_skeleton,
            "poses": poses,
            "frame_rate": scene.render.fps / scene.render.fps_base,
            "keep_source": s.keep_source,
            "curves": curves,
            "counterpart_curves": counterparts,
            "bones": bones,
        }
        try:
            sent, _ = link.call("send_poses", params, preferences.remote_config(context), preferences.project_dir(context))
        except link.LinkError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}

        made = "Created" if sent["created_asset"] else "Updated"
        message = (f"{made} {sent['pose_asset']}: {len(sent['created'])} poses created, {len(sent['updated'])} updated, "
                   f"{len(sent['curves'])} blend shape curves set. Save in Unreal to keep it.")
        if sent["missing_curves"]:
            self.report({"WARNING"}, message + f" No morph target in Unreal for: {', '.join(sent['missing_curves'])}; push the mesh first")
        else:
            self.report({"INFO"}, message)
        return {"FINISHED"}


def _sent_bones(pose):
    """The bones Send Poses keeps for a pose, or None for every bone."""
    if not pose.affected_bones:
        return None
    names = {bone.name for bone in pose.affected_bones}
    if pose.control_bone:
        names.add(pose.control_bone)
    return names


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


class MEATBAG_UL_affected_bones(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index):
        layout.label(text=item.name, icon="BONE_DATA")


class MEATBAG_UL_pose_results(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_property, index):
        row = layout.row(align=True)
        row.operator(MEATBAG_OT_pose_apply.bl_idname, text="", icon="ARMATURE_DATA", emboss=False).index = item.index
        row.label(text=item.name)
        row.prop(item, "before", text="", emboss=False)
        row.prop(item, "after", text="", emboss=False)


classes = (
    MeatbagBone,
    MeatbagPose,
    MeatbagPoseResult,
    MeatbagSettings,
    MEATBAG_OT_pose_add,
    MEATBAG_OT_pose_update,
    MEATBAG_OT_pose_update_control,
    MEATBAG_OT_pose_bones_from_selection,
    MEATBAG_OT_pose_affected_add,
    MEATBAG_OT_pose_affected_remove,
    MEATBAG_OT_pose_remove,
    MEATBAG_OT_pose_move,
    MEATBAG_OT_pose_apply,
    MEATBAG_OT_pose_rest,
    MEATBAG_OT_solve_correctives,
    MEATBAG_OT_sculpt_shape,
    MEATBAG_OT_sculpt_pose,
    MEATBAG_OT_sculpt_back,
    MEATBAG_OT_rebuild_target,
    MEATBAG_OT_send_poses,
    MEATBAG_OT_use_selected_pose_asset,
    MEATBAG_UL_poses,
    MEATBAG_UL_affected_bones,
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
