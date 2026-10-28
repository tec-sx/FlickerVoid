"""FlickerVoid Tools: Blender side of the FlickerVoid character pipeline. See Plan.md next to this folder."""

import bpy

from . import preferences, ui
from .push import operators as push_operators
from .validate import operators as validate_operators

_classes = (
    *preferences.classes,
    *validate_operators.classes,
    *push_operators.classes,
    *ui.classes,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)
    validate_operators.register_properties()
    push_operators.register_properties()


def unregister():
    push_operators.unregister_properties()
    validate_operators.unregister_properties()
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
