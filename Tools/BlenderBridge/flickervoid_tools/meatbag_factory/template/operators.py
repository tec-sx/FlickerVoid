"""Body panel operators: Make Template, Edit Template and Done, Process, and Remove Previous Body."""

import bpy
from bpy.app.handlers import persistent

from . import process
from ... import preferences, scene_data

_states = {}


def cached_state(body):
    """The template state for the panel; the fingerprint is recomputed only after the template changed."""
    if body.name not in _states:
        _states[body.name] = process.state(body)
    return _states[body.name]


def forget_states():
    _states.clear()


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


classes = (
    MEATBAG_OT_make_template,
    MEATBAG_OT_edit_template,
    MEATBAG_OT_template_done,
    MEATBAG_OT_process_template,
    MEATBAG_OT_remove_previous,
)


def register_properties():
    bpy.types.Object.meatbag_template = bpy.props.PointerProperty(
        type=bpy.types.Object, name="Template",
        description="The half mesh with a Mirror modifier that this body is processed from")
    for handlers, handler in _HANDLERS:
        handlers.append(handler)


def unregister_properties():
    for handlers, handler in _HANDLERS:
        if handler in handlers:
            handlers.remove(handler)
    del bpy.types.Object.meatbag_template
