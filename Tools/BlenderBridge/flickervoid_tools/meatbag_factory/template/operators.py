"""Body panel operators: Make Template, Edit Template and Done, Process, Remove Previous Body, and the Detail."""

import bpy
from bpy.app.handlers import persistent

from . import detail as detail_object, process
from ... import preferences, scene_data

_states = {}


def cached_state(body):
    """The template state for the panel; the fingerprint is recomputed only after the template changed."""
    if body.name not in _states:
        _states[body.name] = process.state(body)
    return _states[body.name]


def cached_follows(body):
    """True while the body's Detail shares its topology; recomputed only after a change like the template state."""
    key = ("detail", body.name)
    if key not in _states:
        _states[key] = detail_object.follows(body)
    return _states[key]


def forget_states():
    _states.clear()


def ensure_current(operator, context, body):
    """Processes a changed template before the body is used; reports and returns False when it can't."""
    if process.template_of(body) is None:
        return True
    try:
        report = process.ensure_current(context, body, preferences.contract(context).sides)
    except process.TemplateError as error:
        operator.report({"ERROR"}, str(error))
        return False
    finally:
        forget_states()
    if report is not None:
        operator.report({"INFO"}, report.message(body))
    return True


@persistent
def _template_updated(scene, depsgraph):
    if not _states:
        return
    for update in depsgraph.updates:
        obj = getattr(update.id, "original", update.id)
        if isinstance(obj, bpy.types.Object) and obj.get(process.TEMPLATE_TAG):
            _states.clear()
            return


@persistent
def _reset(*_):
    _states.clear()


_HANDLERS = (
    (bpy.app.handlers.depsgraph_update_post, _template_updated),
    (bpy.app.handlers.load_post, _reset),
    (bpy.app.handlers.undo_post, _reset),
    (bpy.app.handlers.redo_post, _reset),
)


def _body(context):
    return context.scene.meatbag.mesh


def _select_only(context, obj):
    for other in context.selected_objects:
        other.select_set(False)
    obj.select_set(True)
    context.view_layer.objects.active = obj


def _object_mode(context):
    if context.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")


def run_process(operator, context, body):
    """Processes the body and reports; returns the operator result."""
    try:
        report = process.process(context, body, preferences.contract(context).sides)
    except process.TemplateError as error:
        operator.report({"ERROR"}, str(error))
        return {"CANCELLED"}
    finally:
        forget_states()
    operator.report({"WARNING"} if report.projected or report.warnings else {"INFO"}, report.message(body))
    return {"FINISHED"}


class MEATBAG_OT_make_template(bpy.types.Operator):
    bl_idname = "meatbag.make_template"
    bl_label = "Make Template"
    bl_description = ("Turn this half mesh into its template, a hidden copy that keeps the Mirror and the weights, and make "
                      "the body full: its blend shapes and sculpt target poses move to their own side")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        body = _body(context)
        return bool(body and process.mirror_modifier(body) and process.template_of(body) is None and not scene_data.is_helper(body))

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        try:
            report = process.make_template(context, body, preferences.contract(context).sides)
        except process.TemplateError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        finally:
            forget_states()
        template = process.template_of(body)
        text = f"{template.name} made; {body.name} is now the full body with {report.keys} shape keys"
        if report.groups_added:
            text += f". Added to the template so the Mirror can flip them: {', '.join(report.groups_added)}"
        if report.target:
            text += f". {report.target}"
        if report.warnings:
            text += ". " + "; ".join(report.warnings)
        self.report({"WARNING"} if report.warnings else {"INFO"}, text)
        return {"FINISHED"}


class MEATBAG_OT_edit_template(bpy.types.Operator):
    bl_idname = "meatbag.edit_template"
    bl_label = "Edit Template"
    bl_description = "Show the half template in Edit Mode and hide the body. Done processes your changes into the body"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        template = process.template_of(_body(context))
        return bool(template and not process.editing(template))

    def execute(self, context):
        body = _body(context)
        template = process.template_of(body)
        _object_mode(context)
        target = body.meatbag_target
        if target is not None:
            target.hide_set(True)
        template.hide_set(False)
        body.hide_set(True)
        _select_only(context, template)
        bpy.ops.object.mode_set(mode="EDIT")
        forget_states()
        return {"FINISHED"}


class MEATBAG_OT_template_done(bpy.types.Operator):
    bl_idname = "meatbag.template_done"
    bl_label = "Done"
    bl_description = ("Hide the template, show the body again and process the template into it. A topology change projects "
                      "the shape keys and marks the moved vertices in MF_Changed")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        template = process.template_of(_body(context))
        return bool(template and process.editing(template))

    def execute(self, context):
        body = _body(context)
        template = process.template_of(body)
        _object_mode(context)
        template.hide_set(True)
        body.hide_set(False)
        _select_only(context, body)
        return run_process(self, context, body)


class MEATBAG_OT_process_template(bpy.types.Operator):
    bl_idname = "meatbag.process_template"
    bl_label = "Process"
    bl_description = ("Rebuild the body from its template. With the same topology every shape key keeps its shape; with a "
                      "new topology they are projected from the old body, which is kept as <body>_MF_Previous, and the "
                      "moved vertices are marked in MF_Changed")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        template = process.template_of(_body(context))
        return bool(template and not process.editing(template))

    def execute(self, context):
        _object_mode(context)
        return run_process(self, context, _body(context))


class MEATBAG_OT_remove_previous(bpy.types.Operator):
    bl_idname = "meatbag.remove_previous"
    bl_label = "Remove Previous Body"
    bl_description = "Delete the body kept from before the last topology change"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        body = _body(context)
        return bool(body and process.previous_of(body))

    def execute(self, context):
        process.remove_previous(_body(context))
        return {"FINISHED"}


class _DetailOperator:
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return detail_object.detail_of(_body(context)) is not None

    def failed(self, error):
        self.report({"ERROR"}, str(error))
        return {"CANCELLED"}


class MEATBAG_OT_detail_add(bpy.types.Operator):
    bl_idname = "meatbag.detail_add"
    bl_label = "Add Detail"
    bl_description = ("Add the body's Detail: a hidden copy of its base with a Multires modifier, to sculpt skin detail on "
                      "and bake from. It follows the body's shape while the topology stays the same")
    bl_options = {"REGISTER", "UNDO"}

    levels: bpy.props.IntProperty(name="Levels", default=2, min=0, max=6, description="Multires levels to subdivide at once")

    @classmethod
    def poll(cls, context):
        body = _body(context)
        return bool(body and detail_object.detail_of(body) is None and process.mirror_modifier(body) is None
                    and not scene_data.is_helper(body))

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        if not ensure_current(self, context, body):
            return {"CANCELLED"}
        try:
            detail = detail_object.add(body, self.levels)
        except detail_object.DetailError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        finally:
            forget_states()
        self.report({"INFO"}, f"{detail.name} added at multires level {self.levels}. Sculpt Detail to work on it")
        return {"FINISHED"}


class MEATBAG_OT_detail_remove(_DetailOperator, bpy.types.Operator):
    bl_idname = "meatbag.detail_remove"
    bl_label = "Remove Detail"
    bl_description = "Delete the body's Detail and its sculpt"

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event, title="Remove Detail",
                                                     message="Delete the Detail and its sculpt?", confirm_text="Remove")

    def execute(self, context):
        _object_mode(context)
        detail_object.remove(_body(context))
        forget_states()
        return {"FINISHED"}


class MEATBAG_OT_detail_sculpt(_DetailOperator, bpy.types.Operator):
    bl_idname = "meatbag.detail_sculpt"
    bl_label = "Sculpt Detail"
    bl_description = "Hide the body and sculpt its Detail. Back shows the body again"

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        if not ensure_current(self, context, body):
            return {"CANCELLED"}
        detail = detail_object.detail_of(body)
        target = body.meatbag_target
        if target is not None:
            target.hide_set(True)
        detail.hide_set(False)
        body.hide_set(True)
        _select_only(context, detail)
        bpy.ops.object.mode_set(mode="SCULPT")
        return {"FINISHED"}


class MEATBAG_OT_detail_transfer(_DetailOperator, bpy.types.Operator):
    bl_idname = "meatbag.detail_transfer"
    bl_label = "Transfer Detail"
    bl_description = ("Move a Detail left on an older topology onto the body: a new Detail at the same level is projected "
                      "onto the old sculpt along its normals, then the old one is deleted. Optional: the old Detail still bakes")

    @classmethod
    def poll(cls, context):
        return super().poll(context) and not cached_follows(_body(context))

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        try:
            detail = detail_object.transfer(context, body)
        except detail_object.DetailError as error:
            return self.failed(error)
        finally:
            forget_states()
        self.report({"INFO"}, f"{detail.name} moved onto {body.name}'s topology at level {detail_object.levels(detail)}")
        return {"FINISHED"}


class MEATBAG_OT_detail_export(_DetailOperator, bpy.types.Operator):
    bl_idname = "meatbag.detail_export"
    bl_label = "Export Bake Meshes"
    bl_description = ("Write <Body>_low.fbx (the body's base) and <Body>_high.fbx (the Detail at its top level) into the "
                      "Export folder next to the file, for baking in Substance Painter")
    bl_options = {"REGISTER"}

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        if not ensure_current(self, context, body):
            return {"CANCELLED"}
        try:
            paths = detail_object.export_meshes(context, body)
        except (detail_object.DetailError, RuntimeError) as error:
            return self.failed(error)
        self.report({"INFO"}, "Wrote " + ", ".join(paths))
        return {"FINISHED"}


class MEATBAG_OT_detail_bake(_DetailOperator, bpy.types.Operator):
    bl_idname = "meatbag.detail_bake"
    bl_label = "Bake Maps"
    bl_description = ("Bake the Detail's sculpt from its multires levels into UDIM images, one file per tile, in the "
                      "Export folder next to the file")
    bl_options = {"REGISTER"}

    bake_type: bpy.props.EnumProperty(name="Map", items=detail_object.BAKE_TYPES)
    size: bpy.props.EnumProperty(name="Size", default="2048", items=(
        ("1024", "1024", ""), ("2048", "2048", ""), ("4096", "4096", ""), ("8192", "8192", "")))

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

    def execute(self, context):
        body = _body(context)
        _object_mode(context)
        if not ensure_current(self, context, body):
            return {"CANCELLED"}
        try:
            paths = detail_object.bake(context, body, self.bake_type, int(self.size))
        except (detail_object.DetailError, RuntimeError) as error:
            return self.failed(error)
        self.report({"INFO"}, "Wrote " + ", ".join(paths))
        return {"FINISHED"}


classes = (
    MEATBAG_OT_make_template,
    MEATBAG_OT_edit_template,
    MEATBAG_OT_template_done,
    MEATBAG_OT_process_template,
    MEATBAG_OT_remove_previous,
    MEATBAG_OT_detail_add,
    MEATBAG_OT_detail_remove,
    MEATBAG_OT_detail_sculpt,
    MEATBAG_OT_detail_transfer,
    MEATBAG_OT_detail_export,
    MEATBAG_OT_detail_bake,
)


def register_properties():
    bpy.types.Object.meatbag_template = bpy.props.PointerProperty(
        type=bpy.types.Object, name="Template",
        description="The half mesh with a Mirror modifier that this body is processed from")
    bpy.types.Object.meatbag_detail = bpy.props.PointerProperty(
        type=bpy.types.Object, name="Detail",
        description="Hidden multires copy of this body that holds the sculpted detail")
    for handlers, handler in _HANDLERS:
        handlers.append(handler)


def unregister_properties():
    for handlers, handler in _HANDLERS:
        if handler in handlers:
            handlers.remove(handler)
    del bpy.types.Object.meatbag_detail
    del bpy.types.Object.meatbag_template
