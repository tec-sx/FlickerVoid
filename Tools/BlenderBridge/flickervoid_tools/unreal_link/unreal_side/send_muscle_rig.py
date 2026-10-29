import unreal

NODE = "FVMuscleBones"
UNIT = "/Script/FVAnimationSystem.FVRigUnit_MuscleBones"


def _package(path):
    return path.split(".")[0].rstrip("/")


def _quote(name):
    return '"' + name.replace('"', "") + '"'


def _muscles(entries):
    items = [f"(Bone={_quote(e['bone'])},Target={_quote(e['target'])},Volume={float(e['volume'])})" for e in entries]
    return "(" + ",".join(items) + ")"


def _joints(entries):
    items = [f"(Bone={_quote(e['bone'])},Source={_quote(e['source'])},Weight={float(e['weight'])})" for e in entries]
    return "(" + ",".join(items) + ")"


def _create(path):
    folder, _, name = path.rpartition("/")
    factory = unreal.ControlRigBlueprintFactory()
    rig = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.ControlRigBlueprint, factory)
    if rig is None:
        raise RuntimeError(f"Unreal could not create a Control Rig at {path}")
    return rig


def _forward_solve(controller):
    graph = controller.get_graph()
    for node in graph.get_nodes():
        if "BeginExecution" in str(node.get_node_path()) or "BeginExecution" in node.get_class().get_name():
            return str(node.get_node_path())
    for node in graph.get_nodes():
        struct = node.get_script_struct() if hasattr(node, "get_script_struct") else None
        if struct is not None and "BeginExecution" in struct.get_name():
            return str(node.get_node_path())
    node = controller.add_unit_node_from_struct_path("/Script/ControlRig.RigUnit_BeginExecution", "Execute",
                                                     unreal.Vector2D(0.0, 0.0), "BeginExecution")
    return str(node.get_node_path())


def main(params):
    mesh = unreal.load_asset(params["mesh"])
    if not isinstance(mesh, unreal.SkeletalMesh):
        raise ValueError(f"Skeletal mesh not found: {params['mesh']}; push the mesh first")

    path = _package(params["control_rig"])
    rig = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    created = rig is None
    if created:
        rig = _create(path)
    elif not isinstance(rig, unreal.ControlRigBlueprint):
        raise ValueError(f"{path} is a {rig.get_class().get_name()}, not a Control Rig")

    # The rig's bones come from the mesh, so new muscle bones show up after every push.
    rig.set_preview_mesh(mesh)
    rig.get_hierarchy_controller().import_bones_from_asset(params["mesh"], "None", True, True, False, False)

    controller = rig.get_controller_by_name("RigVMModel")
    graph = controller.get_graph()
    node = graph.find_node_by_name(NODE)
    if node is None:
        begin = _forward_solve(controller)
        node = controller.add_unit_node_from_struct_path(UNIT, "Execute", unreal.Vector2D(400.0, 0.0), NODE)
        if node is None:
            raise RuntimeError("Unreal has no FV Muscle Bones node; build the project with the FVAnimationSystem plugin")
        controller.add_link(f"{begin}.ExecuteContext", f"{NODE}.ExecuteContext")
    controller.set_pin_default_value(f"{NODE}.Muscles", _muscles(params["stretch"]), True)
    controller.set_pin_default_value(f"{NODE}.Joints", _joints(params["follow"]), True)
    unreal.BlueprintEditorLibrary.compile_blueprint(rig)
    return {"control_rig": path, "created": created}
