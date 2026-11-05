import bpy

from . import export, mirror
from .. import preferences, scene_data
from ..unreal_link import link
from ..meatbag_factory.contract import checks
from ..meatbag_factory.contract.operators import store_issues, summary
from ..meatbag_factory.template import process
from ..meatbag_factory.template.operators import forget_states


def _call(context, snippet, params=None):
    return link.call(snippet, params, preferences.remote_config(context), preferences.project_dir(context))


def _linkable(context):
    return [obj for obj in scene_data.expand_selection(context.selected_objects, context.scene) if obj.type == "MESH"]


def _push_meshes(context):
    """The selected meshes plus every other mesh linked to the same assets, since an asset is pushed whole."""
    meshes = _linkable(context)
    assets = {mesh.fv_ue_asset for mesh in meshes if mesh.fv_ue_asset}
    meshes += [obj for obj in context.scene.objects if obj.type == "MESH" and obj.fv_ue_asset in assets and obj not in meshes]
    return [mesh for mesh in meshes if not scene_data.is_helper(mesh)]


def _process_templates(context, meshes):
    """Brings every body up to date with its template; returns the names processed. Raises process.TemplateError."""
    processed = []
    try:
        for mesh in meshes:
            if process.template_of(mesh) is not None and process.ensure_current(context, mesh, preferences.contract(context).sides):
                processed.append(mesh.name)
    finally:
        forget_states()
    return processed


def asset_groups(meshes):
    """Groups meshes by linked asset: {asset path: (armature or None, [meshes], skeleton path)}."""
    groups = {}
    for mesh in meshes:
        armature, members, skeleton = groups.setdefault(mesh.fv_ue_asset, (scene_data.armature_of(mesh), [], mesh.fv_ue_skeleton))
        members.append(mesh)
    return groups


def group_problems(groups):
    problems = []
    for asset, (armature, meshes, skeleton) in groups.items():
        names = ", ".join(mesh.name for mesh in meshes)
        if not asset:
            problems.append(f"{names}: not linked to an Unreal asset")
        elif not asset.startswith("/"):
            problems.append(f"{names}: asset path must start with /Game or a plugin mount, got {asset}")
        if len({scene_data.armature_of(mesh) for mesh in meshes}) > 1:
            problems.append(f"{asset}: meshes use different armatures")
        if armature and not skeleton:
            problems.append(f"{asset}: no Skeleton set")
    return problems


class FV_OT_test_connection(bpy.types.Operator):
    bl_idname = "fv.test_connection"
    bl_label = "Test Connection"
    bl_description = "Find the running Unreal Editor and run a command in it"

    def execute(self, context):
        try:
            info, node = _call(context, "editor_info")
        except link.LinkError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Connected to {node.get('project_name', '?')}, Unreal {info['engine_version']}")
        return {"FINISHED"}


class FV_OT_use_selected_asset(bpy.types.Operator):
    bl_idname = "fv.use_selected_asset"
    bl_label = "Use Selected Asset"
    bl_description = "Link the selected meshes to the asset selected in Unreal's Content Browser (a Skeleton sets only the Skeleton)"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return bool(context.selected_objects)

    def execute(self, context):
        meshes = _linkable(context)
        if not meshes:
            self.report({"WARNING"}, "Select meshes or their armature")
            return {"CANCELLED"}
        try:
            asset, _ = _call(context, "selected_asset")
        except link.LinkError as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}

        for mesh in meshes:
            if asset["class"] != "Skeleton":
                mesh.fv_ue_asset = asset["path"]
            mesh.fv_ue_skeleton = asset["skeleton"]
        self.report({"INFO"}, f"Linked {len(meshes)} meshes to {asset['path']}")
        return {"FINISHED"}


class FV_OT_push(bpy.types.Operator):
    bl_idname = "fv.push"
    bl_label = "Push to Unreal"
    bl_description = ("Check the contract, export the selected meshes with the locked FBX preset and import them onto their "
                      "linked Unreal assets. Bodies whose template changed are processed first")
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        return bool(context.selected_objects)

    def execute(self, context):
        meshes = _push_meshes(context)
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        try:
            processed = _process_templates(context, meshes)
        except process.TemplateError as error:
            self.report({"ERROR"}, f"Not pushed: {error}")
            return {"CANCELLED"}
        issues = checks.run(context, meshes)
        store_issues(context, issues)
        if checks.has_errors(issues):
            self.report({"ERROR"}, f"Not pushed: {summary(issues)}. See the Validate panel.")
            return {"CANCELLED"}

        groups = asset_groups(meshes)
        if not groups:
            self.report({"WARNING"}, "Select meshes or their armature")
            return {"CANCELLED"}
        problems = group_problems(groups)
        if problems:
            self.report({"ERROR"}, "Not pushed: " + "; ".join(problems))
            return {"CANCELLED"}

        pushed = []
        for asset, (armature, meshes, skeleton) in groups.items():
            filepath = export.temp_fbx_path(asset)
            try:
                export.export_fbx(context, armature, meshes, filepath)
            except mirror.MirrorError as error:
                self.report({"ERROR"}, str(error))
                return {"CANCELLED"}
            try:
                _call(context, "import_fbx", {"fbx": filepath, "asset": asset, "skeletal": armature is not None, "skeleton": skeleton})
            except link.LinkError as error:
                self.report({"ERROR"}, f"{asset}: {error}")
                return {"CANCELLED"}
            pushed.append(asset)

        warnings = f" ({summary(issues)})" if issues else ""
        refreshed = f" Processed from their templates first: {', '.join(processed)}." if processed else ""
        self.report({"INFO"}, f"Pushed {', '.join(pushed)}{warnings}.{refreshed} Save in Unreal to keep it.")
        return {"FINISHED"}


classes = (FV_OT_test_connection, FV_OT_use_selected_asset, FV_OT_push)


def register_properties():
    bpy.types.Object.fv_ue_asset = bpy.props.StringProperty(
        name="Unreal Asset",
        description="Unreal asset this mesh is pushed to, such as /Game/Characters/Bartender/SK_Bartender. Meshes with the same asset are pushed together",
    )
    bpy.types.Object.fv_ue_skeleton = bpy.props.StringProperty(
        name="Skeleton",
        description="Unreal Skeleton asset a skeletal mesh is imported onto",
    )


def unregister_properties():
    del bpy.types.Object.fv_ue_asset
    del bpy.types.Object.fv_ue_skeleton
