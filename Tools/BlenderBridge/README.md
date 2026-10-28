# FlickerVoid Tools for Blender

One Blender extension for the FlickerVoid character pipeline: push meshes to Unreal and check them against the character contract. The roadmap is in [Plan.md](Plan.md).

Needs Blender 4.2 or later (tested headless on 5.2) and the Unreal Editor running on the same computer.

## 1. Unreal setup

Already set in this project; for another project:

1. Enable the **Python Editor Script Plugin** (`PythonScriptPlugin` in the `.uproject`).
2. *Project Settings > Plugins > Python > Enable Remote Execution* (`bRemoteExecution=True` under `[/Script/PythonScriptPlugin.PythonScriptPluginSettings]` in `Config/DefaultEngine.ini`).
3. Restart the editor.

The defaults (multicast group `239.0.0.1:6766`, bind address `127.0.0.1`) match the add-on's defaults.

## 2. Install in Blender

For development, so edits show after a restart or *Reload Scripts*:

1. *Edit > Preferences > Get Extensions > Repositories > + > Add Local Repository*, directory `Tools/BlenderBridge`.
2. Enable **FlickerVoid Tools** under *Add-ons*.

Or build a zip with `blender --command extension build --source-dir Tools/BlenderBridge/flickervoid_tools` and use *Install from Disk*.

## 3. Preferences

| Setting | Default | Use |
|---|---|---|
| Unreal Project | empty | Folder with the `.uproject`. Side pairs are read from its `Config/DefaultGame.ini` (Animation System settings); the line under it shows which pairs and where from. Also picks the right editor when several are open. |
| Bone Budget | 300 | Most bones (deform bones, as exported) per skeleton |
| Max Influences | 8 | Most bone influences per vertex |
| Unit Scale | 0.01 | Expected scene unit scale (centimetres) |
| Required Bones | Mannequin body chain | Bones every character skeleton must have |
| Remote Execution | UE defaults | Must match the editor's Python plugin settings |

## 4. Use

The **FlickerVoid** tab in the 3D view sidebar.

**Contract.** Select armatures or meshes and press **Check Contract**. Selecting an armature checks the meshes it deforms too. Each row has a select button (object, bones or vertices) and, where it's safe, a fix button (limit influences, normalize). Errors block a push; warnings don't.

**Unreal.**

1. **Test Connection** finds the editor and runs a command in it.
2. Link meshes to an asset: select the asset in the Content Browser, select the meshes (or their armature) in Blender, press **Use Selected Asset**. A Skeletal Mesh sets the asset and its Skeleton; a Skeleton sets only the Skeleton, for a new asset whose path you type in **Asset**.
3. **Push to Unreal** checks the contract, exports each asset with the locked FBX preset and imports it onto its linked path (replacing the existing asset). Meshes linked to the same asset are pushed together, selected or not. Save in Unreal afterwards.

The push exports multires at its base level and the armature in rest pose. It names the armature `Armature` for the export when its top bone is `root`, so Unreal doesn't add an extra root bone. Modifiers that change the vertex count drop shape keys in the FBX.

## 5. Tests

The parts without Blender run with plain Python:

```
python -m unittest discover -s Tools/BlenderBridge/tests
```

## License

GPL-3.0-or-later, as Blender requires for add-ons.
