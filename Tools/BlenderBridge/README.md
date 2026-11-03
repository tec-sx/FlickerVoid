# FlickerVoid Tools for Blender

One Blender extension for the FlickerVoid character pipeline: push meshes to Unreal, check them against the character contract, and solve corrective helper bones (the Meatbag Factory). The roadmap is in [Plan.md](Plan.md).

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

**Meatbag Factory > Contract.** Select armatures or meshes and press **Check Contract**. Selecting an armature checks the meshes it deforms too. Each row has a select button (object, bones or vertices) and, where it's safe, a fix button (limit influences, normalize). Errors block a push; warnings don't.

**Unreal.**

1. **Test Connection** finds the editor and runs a command in it.
2. Link meshes to an asset: select the asset in the Content Browser, select the meshes (or their armature) in Blender, press **Use Selected Asset**. A Skeletal Mesh sets the asset and its Skeleton; a Skeleton sets only the Skeleton, for a new asset whose path you type in **Asset**.
3. **Push to Unreal** checks the contract, exports each asset with the locked FBX preset and imports it onto its linked path (replacing the existing asset). Meshes linked to the same asset are pushed together, selected or not. Save in Unreal afterwards.

The push exports multires at its base level and the armature in rest pose. It names the armature `Armature` for the export when its top bone is `root`, so Unreal doesn't add an extra root bone. Modifiers that change the vertex count drop shape keys in the FBX.

**Meatbag Factory > Poses and Correctives.** Each library pose holds bone corrections and a blend shape, authored by hand, solved from a sculpt, or both. Details in [Plan.md](Plan.md#slice-2-meatbag-factory-correctives).

1. Put the helper bones in a bone collection (`Correctives` by default) and paint their weights. Turn off Preserve Volume on the Armature modifier.
2. In **Poses**, pick the **Armature** and build its pose library: pose the bones in the viewport and press **+** (or **Add Rest Pose** to start from rest), one pose per example, named after the pose (`upperarm_l_up`). Nothing goes on the timeline. Remove, reorder and rename as needed; the weight sliders preview mixes (**Blend** interpolates, **Additive** stacks), blend shapes included. The bone icon shows one pose alone. The **Blend Shape** checkbox in the pose box mutes its `CR_<pose>` key, to judge the bones alone.
3. Each pose has a **Control** bone (the bone whose rotation drives it in Unreal) and **Affected Bones** (the bones it moves). **+** with **Bones from Selection** on, or the **Bones from Selection** button, takes the active bone as control and the other selected bones as affected.
4. **Update Pose** and the refresh button next to **Control** add your viewport changes to the active pose: the affected bones (every bone when the pose has none), or the control bone. Only the change since the library last posed the armature is added, so moving a bone 10 units while two poses are blended adds 10 units to the active pose. The updated bones then show the new mix: in **Blend** a pose at weight 0.5 shows half its change; **Additive** at weight 1 shows it whole. Bones you moved outside the list are reported and not saved.
5. Pick the **Mesh** in **Correctives**. **Sculpt Shape** shows the active pose and opens Sculpt mode on the body's `CR_<pose>` shape key, so you sculpt the blend shape in the pose; press **Back** when done.
6. Or let the solve place the bones: **Sculpt Target** swaps the body for its hidden sculpt target in that pose; sculpt the full shape wanted and press **Back** (with **Solve on Back** that pose is solved at once). For a muscle skin or another existing mesh, pick it as **Other Target** instead. **Solve Helper Bones** solves all unlocked poses or the active one (**Solve**). Each pose solves its own affected bones (the helper collection when it has none), optionally only the selected ones (**Selected Helpers Only**); the control bone and the rest stay as posed; lock poses you tuned by hand. The list shows each pose's error before and after, the `mf_residual` attribute shows where error is left, and **Residual Shape Keys** writes what the bones can't reach into the pose's `CR_<pose>` blend shape, replacing a hand sculpt there.
7. Push the mesh (its `CR_` shape keys become morph targets), set the **Pose Asset** path (or pick the selected one in Unreal with the eyedropper) and press **Send Poses to Unreal**. Poses with a library pose's name are updated, the others created, and the asset is created when missing. With **Affected Bones Only**, each pose sends only its control and affected bones and the rest at rest. A pose with a blend shape gets its `CR_<pose>` curve at 1, which drives the morph target; the control and affected bones are stored as metadata tags on the asset. Save in Unreal.

Sending poses needs the Meatbag Factory commands from PR #66 in the Unreal build.

## 5. Tests

The parts without Blender run with plain Python:

```
python -m unittest discover -s Tools/BlenderBridge/tests
```

## License

GPL-3.0-or-later, as Blender requires for add-ons.
