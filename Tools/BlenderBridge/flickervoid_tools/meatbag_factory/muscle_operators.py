"""Meatbag Factory muscle commands: add between two bones, mirror, remove, and preview the skin on the sculpt target."""

import bpy

from . import anatomy, muscles, sculpt
from .. import preferences


def _settings(context):
    return context.scene.meatbag


def _shape_changed(settings, context):
    muscle = settings.id_data
    if settings.skin is not None and muscle.type == "MESH":
        muscles.build_mesh(muscle)


def _settings_changed(settings, context):
    if settings.skin is not None:
        muscles.apply_settings(settings.id_data)


class MeatbagMuscle(bpy.types.PropertyGroup):
    skin: bpy.props.PointerProperty(type=bpy.types.Object, name="Skin", description="Sculpt target this muscle pushes")
    origin_bone: bpy.props.StringProperty(name="Origin Bone")
    insertion_bone: bpy.props.StringProperty(name="Insertion Bone")
    group: bpy.props.StringProperty(name="Muscle", description="The muscle this strand belongs to")
    length: bpy.props.FloatProperty(name="Length", subtype="DISTANCE", unit="LENGTH", options={"HIDDEN"})
    width: bpy.props.FloatProperty(name="Width", default=3.0, min=0.01, subtype="DISTANCE", unit="LENGTH",
                                   update=_shape_changed, description="Belly radius along the muscle's X axis")
    thickness: bpy.props.FloatProperty(name="Thickness", default=3.0, min=0.01, subtype="DISTANCE", unit="LENGTH",
                                       update=_shape_changed, description="Belly radius along the muscle's Z axis")
    belly: bpy.props.FloatProperty(name="Belly Position", default=0.45, min=0.05, max=0.95, subtype="FACTOR",
                                   update=_shape_changed, description="Where the muscle is widest, from origin to insertion")
    tendon: bpy.props.FloatProperty(name="Tendon", default=0.15, min=0.0, max=1.0, subtype="FACTOR",
                                    update=_shape_changed, description="Radius at the ends, relative to the belly")
    volume: bpy.props.FloatProperty(name="Volume", default=1.0, min=0.0, soft_max=3.0, update=_settings_changed,
                                    description="How much the muscle thickens as it shortens; 1 keeps its volume")
    offset: bpy.props.FloatProperty(name="Skin Offset", default=0.0, subtype="DISTANCE", unit="LENGTH", update=_settings_changed,
                                    description="Skin thickness kept over the muscle")


def _view_changed(view, context):
    muscles.show_all(context.scene)


class MeatbagMuscleView(bpy.types.PropertyGroup):
    visible: bpy.props.BoolProperty(name="Show Muscles", default=True, update=_view_changed)
    opacity: bpy.props.FloatProperty(name="Opacity", default=0.5, min=0.0, max=1.0, subtype="FACTOR", update=_view_changed,
                                     description="Muscle opacity in Solid shading (Color: Material)")
    solid: bpy.props.BoolProperty(name="Solid", default=True, update=_view_changed,
                                  description="Draw muscles as coloured surfaces, else as wireframes")
    in_front: bpy.props.BoolProperty(name="In Front", default=False, update=_view_changed,
                                     description="Draw muscles over the body")


class MEATBAG_OT_muscles_generate(bpy.types.Operator):
    bl_idname = "meatbag.muscles_generate"
    bl_label = "Generate Muscles"
    bl_description = ("Build the human muscle template on a Mannequin skeleton, fitted under the skin, on both sides. "
                      "Replaces muscles generated before; hand-added muscles stay")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = _settings(context)
        return bool(s.armature and s.mesh)

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def execute(self, context):
        s = _settings(context)
        missing = anatomy.missing_bones(s.armature.data.bones.keys())
        if missing:
            self.report({"ERROR"}, f"Not a Mannequin skeleton; missing {', '.join(missing[:6])}")
            return {"CANCELLED"}
        try:
            skin = sculpt.ensure_target(s.mesh)
            if context.mode != "OBJECT":
                bpy.ops.object.mode_set(mode="OBJECT")
            created = muscles.generate(context, s.armature, skin, preferences.contract(context).sides, anatomy.TEMPLATE)
        except (muscles.MuscleError, sculpt.SculptError) as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Generated {len(created)} muscle strands; Preview Skin to see the skin")
        return {"FINISHED"}


class MEATBAG_OT_muscles_fit(bpy.types.Operator):
    bl_idname = "meatbag.muscles_fit"
    bl_label = "Fit to Skin"
    bl_description = ("Reshape every muscle to the room under the skin at rest: each ring follows the distance to the "
                      "skin around it. Run it again after moving anchors or changing the body")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = _settings(context)
        return bool(s.armature and s.mesh and s.mesh.meatbag_target)

    def execute(self, context):
        s = _settings(context)
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        fitted = muscles.fit_all(context, s.armature, s.mesh.meatbag_target)
        self.report({"INFO"}, f"Fitted {len(fitted)} muscles")
        return {"FINISHED"}


def _selected_bones(context, armature):
    """(origin, insertion) from pose mode: the other selected bone and the active bone."""
    if context.mode != "POSE" or context.active_object != armature:
        return "", ""
    active = context.active_pose_bone
    others = [bone.name for bone in context.selected_pose_bones or [] if bone != active]
    return (others[0] if others else ""), (active.name if active else "")


class MEATBAG_OT_muscle_add(bpy.types.Operator):
    bl_idname = "meatbag.muscle_add"
    bl_label = "Add Muscle"
    bl_description = ("Add a muscle from the origin bone to the insertion bone (in Pose mode: select the origin, then the "
                      "insertion as active). It pushes the sculpt target's skin, which is made when missing")
    bl_options = {"REGISTER", "UNDO"}

    name: bpy.props.StringProperty(name="Name", default="muscle_l")
    origin_bone: bpy.props.StringProperty(name="Origin")
    insertion_bone: bpy.props.StringProperty(name="Insertion")
    origin_at: bpy.props.FloatProperty(name="Origin Along Bone", default=0.2, min=0.0, max=1.0, subtype="FACTOR")
    insertion_at: bpy.props.FloatProperty(name="Insertion Along Bone", default=0.2, min=0.0, max=1.0, subtype="FACTOR")

    @classmethod
    def poll(cls, context):
        s = _settings(context)
        return bool(s.armature and s.mesh)

    def invoke(self, context, event):
        origin, insertion = _selected_bones(context, _settings(context).armature)
        self.origin_bone = self.origin_bone or origin
        self.insertion_bone = self.insertion_bone or insertion
        return context.window_manager.invoke_props_dialog(self)

    def draw(self, context):
        layout = self.layout
        armature = _settings(context).armature
        layout.prop(self, "name")
        layout.prop_search(self, "origin_bone", armature.data, "bones")
        layout.prop(self, "origin_at")
        layout.prop_search(self, "insertion_bone", armature.data, "bones")
        layout.prop(self, "insertion_at")

    def execute(self, context):
        s = _settings(context)
        try:
            skin = sculpt.ensure_target(s.mesh)
            origin = muscles.bone_point(s.armature, self.origin_bone, self.origin_at)
            insertion = muscles.bone_point(s.armature, self.insertion_bone, self.insertion_at)
            mode = context.mode
            if mode != "OBJECT":
                bpy.ops.object.mode_set(mode="OBJECT")
            muscle = muscles.create(context, s.armature, skin, self.name, self.origin_bone, self.insertion_bone, origin, insertion)
        except (muscles.MuscleError, sculpt.SculptError) as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        for obj in context.selected_objects:
            obj.select_set(False)
        muscle.select_set(True)
        context.view_layer.objects.active = muscle
        self.report({"INFO"}, f"Added {muscle.name}; move its anchors to place it, Preview Skin to see the skin")
        return {"FINISHED"}


def _active_muscle(context):
    obj = context.active_object
    return obj if obj and obj.type == "MESH" and obj.meatbag_muscle.skin is not None else None


class MEATBAG_OT_muscle_mirror(bpy.types.Operator):
    bl_idname = "meatbag.muscle_mirror"
    bl_label = "Mirror Muscle"
    bl_description = "Make or replace the active muscle's counterpart on the other side, using the contract's side names"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return _active_muscle(context) is not None

    def execute(self, context):
        try:
            created = muscles.mirror(context, _active_muscle(context), preferences.contract(context).sides)
        except muscles.MuscleError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Mirrored to {created.name}")
        return {"FINISHED"}


class MEATBAG_OT_muscle_remove(bpy.types.Operator):
    bl_idname = "meatbag.muscle_remove"
    bl_label = "Remove Muscle"
    bl_description = "Remove the active muscle, its anchors and its skin modifier"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return _active_muscle(context) is not None

    def execute(self, context):
        muscle = _active_muscle(context)
        name = muscle.name
        muscles.remove(muscle)
        self.report({"INFO"}, f"Removed {name}")
        return {"FINISHED"}


class MEATBAG_OT_muscle_select(bpy.types.Operator):
    bl_idname = "meatbag.muscle_select"
    bl_label = "Select Muscle"
    bl_description = "Select this muscle"

    name: bpy.props.StringProperty()

    def execute(self, context):
        muscle = bpy.data.objects.get(self.name)
        if muscle is None:
            return {"CANCELLED"}
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        for obj in context.selected_objects:
            obj.select_set(False)
        muscle.hide_set(False)
        muscle.select_set(True)
        context.view_layer.objects.active = muscle
        return {"FINISHED"}


class MEATBAG_OT_preview_skin(bpy.types.Operator):
    bl_idname = "meatbag.preview_skin"
    bl_label = "Preview Skin"
    bl_description = "Swap the body and the sculpt target in the viewport, to see the muscles push the skin"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        s = _settings(context)
        return bool(s.mesh and s.mesh.meatbag_target)

    def execute(self, context):
        s = _settings(context)
        target = s.mesh.meatbag_target
        showing = not target.hide_get()
        if context.mode not in {"OBJECT", "POSE"}:
            bpy.ops.object.mode_set(mode="OBJECT")
        target.hide_set(showing)
        s.mesh.hide_set(not showing)
        return {"FINISHED"}


classes = (
    MeatbagMuscle,
    MeatbagMuscleView,
    MEATBAG_OT_muscles_generate,
    MEATBAG_OT_muscles_fit,
    MEATBAG_OT_muscle_add,
    MEATBAG_OT_muscle_mirror,
    MEATBAG_OT_muscle_remove,
    MEATBAG_OT_muscle_select,
    MEATBAG_OT_preview_skin,
)


def register_properties():
    bpy.types.Object.meatbag_muscle = bpy.props.PointerProperty(type=MeatbagMuscle)
    bpy.types.Scene.meatbag_muscles = bpy.props.PointerProperty(type=MeatbagMuscleView)


def unregister_properties():
    del bpy.types.Object.meatbag_muscle
    del bpy.types.Scene.meatbag_muscles
