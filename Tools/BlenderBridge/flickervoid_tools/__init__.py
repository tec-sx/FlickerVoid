"""FlickerVoid Tools: Blender side of the FlickerVoid character pipeline. See Plan.md next to this folder."""

import bpy

from . import preferences, ui
from .push import operators as push_operators
from .meatbag_factory import muscle_operators
from .meatbag_factory import operators as meatbag_operators
from .meatbag_factory.contract import operators as contract_operators

_classes = (
    *preferences.classes,
    *contract_operators.classes,
    *meatbag_operators.classes,
    *muscle_operators.classes,
    *push_operators.classes,
    *ui.classes,
)


def register():
    for cls in _classes:
        bpy.utils.register_class(cls)
    contract_operators.register_properties()
    meatbag_operators.register_properties()
    muscle_operators.register_properties()
    push_operators.register_properties()


def unregister():
    push_operators.unregister_properties()
    muscle_operators.unregister_properties()
    meatbag_operators.unregister_properties()
    contract_operators.unregister_properties()
    for cls in reversed(_classes):
        bpy.utils.unregister_class(cls)
