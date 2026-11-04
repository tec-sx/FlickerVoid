# FlickerVoid Tools for Blender

One Blender extension for the FlickerVoid character pipeline: keep characters in a library (the Meat Locker), build full bodies from half templates, sculpt and bake detail, solve corrective helper bones (the Meatbag Factory), check them against the character contract and push them to Unreal. The roadmap is in [Plan.md](Plan.md).

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
| Character Library | empty | The Meat Locker's folder: one folder per character holding `<Name>.blend`. The button next to it adds the folder to Blender's asset libraries. |
| Bone Budget | 300 | Most bones (deform bones, as exported) per skeleton |
| Max Influences | 8 | Most bone influences per vertex |
| Unit Scale | 0.01 | Expected scene unit scale (centimetres) |
| Required Bones | Mannequin body chain | Bones every character skeleton must have; `root` is the armature itself and is ignored here |
| Remote Execution | UE defaults | Must match the editor's Python plugin settings |

## 4. Use

The **FlickerVoid** tab in the 3D view sidebar.

**Meat Locker.** The character library, one `.blend` per character. Pick its folder at the top of the panel. The list shows each character's saved thumbnail, Base or Hero, the Reference (armature icon) and the open file (file icon).

- **+** adds a character and opens it. It starts from **Empty** (a new file with the contract's units and the Reference character's armature), **This File** (the open file saved into the library, the way to bring in an existing character) or **Selected Character**. The copy button starts from the selected character as a Hero.
- A copy records the character it came from and drops its Unreal asset links, so its push can't overwrite the original's asset.
- **Open** opens the selected character, and **Edit Template** opens it on its half template. Unsaved changes in the open file are saved first unless you untick **Save Current File**.
- The trash button moves the selected character's folder into `_Trash`.
- **Base** and **Hero** set the open character's kind; save to keep it. **Reference** marks the character whose skeleton the others are checked against (stored in `MeatLocker.json` in the library folder).
- Each character's collection is named after it and marked as an asset in the *Meat Locker/Base* or *Meat Locker/Hero* catalog, so once the folder is an asset library, garment files can drag a body in from the Asset Browser. A character's kind, base and armature are in its `MF_Character` text block.

**Meatbag Factory > Body.** The **Body** is the skinned character mesh that the poses, sculpts, solve and push work on. A symmetric body is built from a **template**, a half mesh that is the only one you model.

1. Model one half (left is +X) with a **Mirror** modifier: X only, **Merge** on, no mirror object. Lay its UVs in the left half of each UDIM tile (U 0 to 0.5 within the tile, such as the head in 1001 and the body in 1002) and keep them off the tile's left edge; the Mirror flips them into the right half of the same tile. Paint the weights on the half; the Mirror flips `_l` groups into `_r` groups.
2. Pick the half mesh as the **Body** and press **Make Template**. A hidden copy, `<Body>_Template`, becomes the template and keeps the Mirror and the weights; the body becomes the full mesh with the Mirror applied. Shape keys and sculpt target poses with a side token move to their own side; the others stay on both sides. Missing `_r` groups are added to the template, since the Mirror only flips weights into groups that exist.
3. Change the shape, UVs and weights only on the template. **Edit Template** hides the body and opens the template in Edit Mode (switch to Weight Paint there to paint); **Done** hides it again and processes it into the body. Weights painted on the body are replaced at the next Process.
4. **Process** rebuilds the body from its template. With the same topology nothing is lost: every shape key, the sculpt target and the body's own attributes (such as `mf_residual`) carry over vertex by vertex. After a topology change, the shape keys and sculpt target poses are projected from the old body, the vertices with no old vertex at their place are marked in the `MF_Changed` vertex group, and the old body is kept, hidden, as `<Body>_MF_Previous` to compare with; the trash button deletes it.

A template changed outside Edit Template is processed on its own before Sculpt Shape, Sculpt Target, Solve and Push when its topology is the same; after a topology change they wait for **Process**. Process sets the Mirror's UV and vertex group flips itself. Solved weights are written back into the template (see step 6 below). Each body vertex stores the template vertex it came from (`mf_template_index`) and its mirror partner (`mf_mirror`).

**Detail**, in the same panel, is for sculpted skin detail and texture bakes:

- **Add Detail** makes `<Body>_MF_Detail`, a hidden copy of the full body with a Multires modifier. **Sculpt Detail** shows it in Sculpt mode, and **Back** returns to the body.
- While the body keeps its topology, Process moves the Detail's base along and the sculpt follows. After a topology change the Detail stays on the old topology, which still bakes, until **Transfer Detail** projects its sculpt onto a new Detail along the normals.
- **Export Bake Meshes** writes `<Body>_low.fbx` (the body) and `<Body>_high.fbx` (the Detail at its top level) into an `Export` folder next to the file, for Substance Painter.
- **Bake Maps** bakes a normal, displacement or vector displacement map from the multires levels into `<Body>_<map>.<tile>.png` (or `.exr`), one file per UDIM tile, in the same folder.

**Meatbag Factory > Validate.** Select armatures or meshes and press **Validate**. Selecting an armature checks the meshes it deforms too. Each row has a select button (object, bones or vertices) and, where it's safe, a fix button. Errors block a push; warnings don't. Under **Validate**, **Remove Unused Groups**, **Limit Total** and **Normalize** fix the weights of the selected meshes (or of a selected armature's meshes) and validate again. Remove Unused Groups removes empty vertex groups and groups matching no deform bone, but keeps groups used by modifiers or shape keys, `MF_` groups and a template's empty counterpart groups, which its Mirror fills. Limit Total keeps each vertex's largest deform influences up to **Max Influences**; Normalize makes them sum to 1. A body made from a template gets its weights from the template, so the fixes change the template and process the body. When the library has a Reference character, **Check Skeleton** compares the armature picked in Poses (or the active one) with the Reference's deform bones and lists other parents (errors), missing bones, other rolls and extra bones. **Sync Skeleton** adds the missing bones, placed relative to their parent and scaled by its length, in the same bone collections; paint or solve their weights afterwards.

**Unreal.**

1. **Test Connection** finds the editor and runs a command in it.
2. Link meshes to an asset: select the asset in the Content Browser, select the meshes (or their armature) in Blender, press **Use Selected Asset**. A Skeletal Mesh sets the asset and its Skeleton; a Skeleton sets only the Skeleton, for a new asset whose path you type in **Asset**.
3. **Push to Unreal** checks the contract, exports each asset with the locked FBX preset and imports it onto its linked path (replacing the existing asset). Meshes linked to the same asset are pushed together, selected or not. Save in Unreal afterwards.

The push exports multires at its base level and the armature in rest pose. Unreal makes the armature object the skeleton's root bone, so the push exports it named `root` whatever its name in Blender, and every character's skeleton gets the same root. The armature therefore has no `root` bone: **Validate** reports one as an error, and its fix deletes the bone and leaves its children at the top. Modifiers that change the vertex count drop shape keys in the FBX, except a **Mirror** modifier (X, no mirror object): such a mesh is pushed as a temporary full copy with the Mirror applied, where a shape key with a side token (`CR_upperarm_l_up`) moves only its side and its counterpart (`CR_upperarm_r_up`) is made by reflection unless the mesh has one; keys without a side token stay symmetric. Left is +X. A body built from a template is already full: its copy only gets each side key's missing counterpart, reflected through the stored mirror partners, and the template itself is never pushed.

**Meatbag Factory > Poses and Correctives.** Each library pose holds bone corrections and a blend shape, authored by hand, solved from a sculpt, or both. Details in [Plan.md](Plan.md#slice-2-meatbag-factory-correctives).

1. Put the helper bones in a bone collection (`Correctives` by default) and paint their weights. Turn off Preserve Volume on the Armature modifier.
2. In **Poses**, pick the **Armature** and build its pose library: pose the bones in the viewport and press **+** (or **Add Rest Pose** to start from rest), one pose per example, named after the pose (`upperarm_l_up`). Nothing goes on the timeline. Remove, reorder and rename as needed; the weight sliders preview mixes (**Blend** interpolates, **Additive** stacks), blend shapes included. The bone icon shows one pose alone. The **Blend Shape** checkbox in the pose box mutes its `CR_<pose>` key, to judge the bones alone.
3. Each pose has a **Control** bone (the bone whose rotation drives it in Unreal) and **Affected Bones** (the bones it moves). **+** with **Bones from Selection** on, or the **Bones from Selection** button, takes the active bone as control and the other selected bones as affected.
4. **Update Control Pose** and **Update Affected Bones** add your viewport changes to the active pose: the affected bones (every bone when the pose has none), or the control bone. Only the change since the library last posed the armature is added, so moving a bone 10 units while two poses are blended adds 10 units to the active pose. The updated bones then show the new mix: in **Blend** a pose at weight 0.5 shows half its change; **Additive** at weight 1 shows it whole. Bones you moved outside the list are reported and not saved.
5. With the **Body** picked, **Sculpt Shape** shows the active pose and opens Sculpt mode on the body's `CR_<pose>` shape key, so you sculpt the blend shape in the pose; press **Back** when done. X symmetry is on only for centre poses, so a side pose is sculpted on its own side; Back restores your setting. On a half mesh without a template, the Mirror modifier is hidden while you sculpt, since the push puts the shape on its own side.
6. Or let the solve place the bones: **Sculpt Target** swaps the body for its hidden sculpt target in that pose, with the same symmetry rule; sculpt the full shape wanted and press **Back** (with **Solve on Back** that pose is solved at once). **Solve Helper Bones** solves all unlocked poses or the active one (**Solve**). Each pose solves its own affected bones (the helper collection when it has none), optionally only the selected ones (**Selected Helpers Only**); the control bone and the rest stay as posed; lock poses you tuned by hand. The list shows each pose's error before and after, the `mf_residual` attribute shows where error is left, and **Residual Shape Keys** writes what the bones can't reach into the pose's `CR_<pose>` blend shape, replacing a hand sculpt there. On a body with a template, **Solve Weights** writes the solved weights back into the template and processes it, so both sides get them; where a vertex and its mirror partner both changed, the template's side wins.
7. Push the mesh (its `CR_` shape keys become morph targets), set the **Pose Asset** path (or pick the selected one in Unreal with the eyedropper) and press **Send Poses to Unreal**. Poses with a library pose's name are updated, the others created, and the asset is created when missing. With **Affected Bones Only**, each pose sends only its control and affected bones and the rest at rest. A pose with a blend shape gets its `CR_<pose>` curve at 1, which drives the morph target, and the counterpart curve (`CR_..._r`) is added at 0 so **Mirror All** fills it; the control and affected bones are stored as metadata tags on the asset. Save in Unreal.

Sending poses needs the Meatbag Factory commands from PR #66 in the Unreal build.

## 5. Tests

The parts without Blender run with plain Python:

```
python -m unittest discover -s Tools/BlenderBridge/tests
```

## License

GPL-3.0-or-later, as Blender requires for add-ons.
