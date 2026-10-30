import bpy

from .core.contract import MANNEQUIN_BODY_CHAIN, Contract, load_side_pairs, parse_bone_list
from .unreal_link.remote import RemoteConfig


def _endpoint(text, default):
    host, _, port = text.rpartition(":")
    try:
        return (host or default[0], int(port))
    except ValueError:
        return default


class FVToolsPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__

    project_dir: bpy.props.StringProperty(
        name="Unreal Project",
        subtype="DIR_PATH",
        description="Folder of the Unreal project (the one holding the .uproject). Contract values such as side pairs are read from its Config",
    )
    bone_budget: bpy.props.IntProperty(name="Bone Budget", default=300, min=1, description="Most bones a skeleton may have")
    max_influences: bpy.props.IntProperty(name="Max Influences", default=8, min=1, max=12, description="Most bone influences per vertex")
    unit_scale: bpy.props.FloatProperty(name="Unit Scale", default=0.01, precision=4, description="Scene unit scale the contract expects (0.01 is centimetres)")
    required_bones: bpy.props.StringProperty(
        name="Required Bones",
        default=", ".join(MANNEQUIN_BODY_CHAIN),
        description="Comma-separated bones every character skeleton must have",
    )

    multicast_group: bpy.props.StringProperty(name="Multicast Group", default="239.0.0.1:6766", description="Must match Multicast Group Endpoint in the Unreal Python settings")
    multicast_bind_address: bpy.props.StringProperty(name="Multicast Bind Address", default="127.0.0.1", description="Must match Multicast Bind Address in the Unreal Python settings")
    command_endpoint: bpy.props.StringProperty(name="Command Endpoint", default="127.0.0.1:6776", description="Address the editor connects back to for commands")
    command_timeout: bpy.props.FloatProperty(name="Command Timeout", default=600.0, min=5.0, subtype="TIME_ABSOLUTE", description="Seconds to wait for an import or export in Unreal")

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "project_dir")
        pairs, source = load_side_pairs(bpy.path.abspath(self.project_dir))
        layout.label(text="Side pairs: " + ", ".join(f"{left}/{right}" for left, right in pairs) + f"  ({source})")

        box = layout.box()
        box.label(text="Contract")
        box.prop(self, "bone_budget")
        box.prop(self, "max_influences")
        box.prop(self, "unit_scale")
        box.prop(self, "required_bones")

        box = layout.box()
        box.label(text="Unreal Remote Execution")
        box.prop(self, "multicast_group")
        box.prop(self, "multicast_bind_address")
        box.prop(self, "command_endpoint")
        box.prop(self, "command_timeout")


def get(context=None):
    return (context or bpy.context).preferences.addons[__package__].preferences


def project_dir(context=None):
    path = get(context).project_dir
    return bpy.path.abspath(path) if path else ""


def contract(context=None):
    prefs = get(context)
    pairs, source = load_side_pairs(project_dir(context))
    return Contract(
        side_pairs=pairs,
        required_bones=parse_bone_list(prefs.required_bones),
        bone_budget=prefs.bone_budget,
        max_influences=prefs.max_influences,
        unit_scale=prefs.unit_scale,
        side_pairs_source=source,
    )


def remote_config(context=None):
    prefs = get(context)
    defaults = RemoteConfig()
    return RemoteConfig(
        multicast_group=_endpoint(prefs.multicast_group, defaults.multicast_group),
        multicast_bind_address=prefs.multicast_bind_address or defaults.multicast_bind_address,
        command_endpoint=_endpoint(prefs.command_endpoint, defaults.command_endpoint),
        command_timeout=prefs.command_timeout,
    )


classes = (FVToolsPreferences,)
