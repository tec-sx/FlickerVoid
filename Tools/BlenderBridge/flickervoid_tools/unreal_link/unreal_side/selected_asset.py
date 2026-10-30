import unreal


def main(params):
    assets = unreal.EditorUtilityLibrary.get_selected_assets()
    if len(assets) != 1:
        raise ValueError(f"Select exactly one asset in the Content Browser ({len(assets)} selected)")

    asset = assets[0]
    result = {"path": asset.get_path_name().split(".")[0], "class": asset.get_class().get_name(), "skeleton": ""}
    if isinstance(asset, unreal.SkeletalMesh):
        skeleton = asset.get_editor_property("skeleton")
        result["skeleton"] = skeleton.get_path_name().split(".")[0] if skeleton else ""
    elif isinstance(asset, unreal.Skeleton):
        result["skeleton"] = result["path"]
    return result
