import unreal


def _package(path):
    return path.split(".")[0].rstrip("/")


def _import_animation(params, skeleton):
    options = unreal.FbxImportUI()
    options.import_mesh = False
    options.import_animations = True
    options.import_materials = False
    options.import_textures = False
    options.create_physics_asset = False
    options.automated_import_should_detect_type = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    options.skeleton = skeleton
    data = options.anim_sequence_import_data
    data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    data.set_editor_property("import_bone_tracks", True)
    data.set_editor_property("import_meshes_in_bone_hierarchy", False)
    data.set_editor_property("import_custom_attribute", False)
    data.set_editor_property("remove_redundant_keys", False)
    data.set_editor_property("use_default_sample_rate", False)
    data.set_editor_property("custom_sample_rate", params["frame_rate"])

    folder, _, name = _package(params["pose_asset"]).rpartition("/")
    task = unreal.AssetImportTask()
    task.filename = params["fbx"]
    task.destination_path = folder
    task.destination_name = name + "_Source"
    task.replace_existing = True
    task.replace_existing_settings = False
    task.automated = True
    task.save = False
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    for path in task.imported_object_paths:
        asset = unreal.load_asset(str(path))
        if isinstance(asset, unreal.AnimSequence):
            return asset
    raise RuntimeError("Unreal imported no animation; see the editor's Output Log")


def _set_curves(pose_asset, curves):
    """Sets each pose's blend shape curve to 1, adding the curve to the asset first; returns (set, missing)."""
    library = unreal.FVPoseAuthoringLibrary
    done, missing = [], []
    for pose, curve in curves.items():
        addable = [str(name) for name in library.get_addable_curves(pose_asset)]
        # Bool functions with out parameters return None on failure in Python.
        if curve in addable and library.add_curve(pose_asset, curve) is None:
            missing.append(curve)
            continue
        if library.set_curve_value(pose_asset, pose, curve, 1.0) is None:
            missing.append(curve)
        else:
            done.append(curve)
    return done, missing


def _set_bone_tags(pose_asset, bones):
    """Stores each pose's control and affected bones as editor metadata (MeatbagFactory.<pose>.Control / .Affected)."""
    for pose, entry in bones.items():
        unreal.EditorAssetLibrary.set_metadata_tag(pose_asset, f"MeatbagFactory.{pose}.Control", entry["control"])
        unreal.EditorAssetLibrary.set_metadata_tag(pose_asset, f"MeatbagFactory.{pose}.Affected", ",".join(entry["affected"]))


def main(params):
    skeleton = unreal.load_asset(params["skeleton"])
    if not isinstance(skeleton, unreal.Skeleton):
        raise ValueError(f"Skeleton not found: {params['skeleton']}")

    path = _package(params["pose_asset"])
    pose_asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    created_asset = False
    if pose_asset is None:
        mesh = unreal.load_asset(params["mesh"])
        if not isinstance(mesh, unreal.SkeletalMesh):
            raise ValueError(f"Skeletal mesh not found: {params['mesh']}; push the mesh first")
        pose_asset, error = unreal.FVPoseAuthoringLibrary.create_pose_asset(path, mesh)
        if pose_asset is None:
            raise RuntimeError(str(error))
        created_asset = True
    elif not isinstance(pose_asset, unreal.PoseAsset):
        raise ValueError(f"{path} is a {pose_asset.get_class().get_name()}, not a Pose Asset")

    animation = _import_animation(params, skeleton)
    try:
        result = unreal.FVPoseAuthoringLibrary.set_poses_from_animation(pose_asset, animation, params["poses"])
    finally:
        if not params.get("keep_source"):
            unreal.EditorAssetLibrary.delete_asset(_package(animation.get_path_name()))

    if not result.get_editor_property("succeeded"):
        if created_asset:
            unreal.EditorAssetLibrary.delete_asset(path)
        raise RuntimeError(str(result.get_editor_property("error")))
    curves, missing = _set_curves(pose_asset, params.get("curves", {}))
    _set_bone_tags(pose_asset, params.get("bones", {}))
    return {
        "pose_asset": path,
        "created_asset": created_asset,
        "created": [str(name) for name in result.get_editor_property("created")],
        "updated": [str(name) for name in result.get_editor_property("updated")],
        "curves": curves,
        "missing_curves": missing,
    }
