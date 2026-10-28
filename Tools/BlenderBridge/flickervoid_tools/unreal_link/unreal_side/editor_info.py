import unreal


def main(params):
    return {
        "engine_version": unreal.SystemLibrary.get_engine_version(),
        "project": unreal.Paths.get_project_file_path(),
    }
