import unreal


def main(params):
    package_path, _, asset_name = params["asset"].rpartition("/")

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = params["skeletal"]
    options.import_materials = False
    options.import_textures = False
    options.import_animations = False
    options.create_physics_asset = False
    options.automated_import_should_detect_type = False

    normals = unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
    if params["skeletal"]:
        options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
        skeleton = unreal.load_asset(params["skeleton"])
        if not isinstance(skeleton, unreal.Skeleton):
            raise ValueError(f"Skeleton not found: {params['skeleton']}")
        options.skeleton = skeleton
        data = options.skeletal_mesh_import_data
        data.set_editor_property("import_morph_targets", True)
        data.set_editor_property("normal_import_method", normals)
        data.set_editor_property("use_t0_as_ref_pose", False)
    else:
        options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
        data = options.static_mesh_import_data
        data.set_editor_property("combine_meshes", True)
        data.set_editor_property("normal_import_method", normals)

    task = unreal.AssetImportTask()
    task.filename = params["fbx"]
    task.destination_path = package_path
    task.destination_name = asset_name
    task.replace_existing = True
    task.replace_existing_settings = False
    task.automated = True
    task.save = False
    task.options = options

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = [str(path) for path in task.imported_object_paths]
    if not imported:
        raise RuntimeError("Unreal imported nothing; see the editor's Output Log")
    return {"imported": imported}
