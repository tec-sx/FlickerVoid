# FVAnimationSystem setup

Left/right naming rules and the Meatbag Factory, the editor window that authors and mirrors poses in Pose Assets (the target poses read by the corrective rig in the post-process Anim Blueprint). It edits stock Pose Assets, adds no data to them and needs no runtime setup.

Modules: `FVAnimationSystem` (settings, side naming, `UFVAnimationLibrary`), `FVAnimationSystemDebug` (empty for now), `FVAnimationSystemEditor` (Meatbag Factory window, pose commands, mirroring, Pose Asset validator, `UFVPoseAuthoringLibrary`).

## 1. Quick start

1. Open a skeletal mesh and choose *Window > Meatbag Factory* (also in the Skeleton editor).
2. In **Setup**, pick a Pose Asset on the mesh's skeleton. Make one first with *Create Asset > Pose Asset* in the toolbar if needed. The Pose Asset (or its skeleton) needs a **Preview Mesh**.
3. Select a bone, rotate it in the viewport, then **Create** and name the pose with its side: `upperarm_l_up_90`.
4. Author the whole left side, then press **Mirror All L > R** once: every right pose is updated if it exists and created if not. Ctrl+Z undoes any command.

## 2. The window

| Section | Holds |
|---|---|
| **Setup** | **Pose Asset** picker (filtered to the skeleton), and two overrides for this session that start at the project defaults: **Mirror Axis** (for creatures or robots whose pairs mirror on another axis) and **Authoring Side** |
| **Poses** | Filter, the pose list (side badge L, R or C; double-click a name to rename; a preview weight slider per pose), and the selected pose's curve values with **Add Curve** |
| **Actions** | Create, Update, Delete · Go To, Reset, Duplicate · Mirror All (arrow: mirror selected poses only), Inbetweens, Copy/Paste |

How capture works: **Create** and **Update** save the pose last opened with **Go To** (or double-click), or the base pose after **Reset**, plus the bones you edited in the viewport. Post-process correctives showing on the preview are never baked in. Preview weight sliders only change the viewport.

- **Mirror All** mirrors every authoring-side pose in one undo step. When other-side poses have lost their authoring-side pose (renamed or deleted), it lists them and asks whether to delete them in the same step. A notification then reports how many poses were created, updated and skipped, with each skipped pose and why (also in the Output Log).
- **Inbetweens**: 1 to 9 poses between the base pose and the selected pose, named `<pose>_ib<percent>` (`thigh_l_raise_ib50`).
- **Copy/Paste**: copy a pose, or only the bones selected in the skeleton tree, and paste onto the selected poses with a **Multiplier** (1 as copied, 0.5 half the change from the base pose, 1.5 exaggerated). A bone-only copy pastes no curves.
- **Add Curve**: lists the skeleton's curves (authored in content, *Skeleton > Anim Curves*) that the asset lacks. Adding one rebuilds the asset through a temporary animation and keeps every pose, its name and the additive setup; the asset's own Source Animation is left as it was.
- Disabled buttons say why in their tooltip.

## 3. Settings

*Project Settings > FlickerVoid > Animation System*:

| Setting | Default | Use |
|---|---|---|
| **Side Pairs** | `_l` / `_r` | Bone suffixes that mark a side; matched ignoring case, case kept (`_L` becomes `_R`). Pose and curve names use the same pairs as whole tokens. |
| **Default Mirror Axis** | X | Component-space axis that poses are mirrored across. The window overrides it per session, and scripting passes it per call (`EFVMirrorAxis`). |
| **Authoring Side** | Left | The side you author; Mirror All copies it to the other side. The window overrides it per session. |
| **Changed Rotation / Translation / Scale Tolerance** | 0.01 deg / 0.01 cm / 0.001 | When a bone counts as changed by a pose, compared with the base pose (the reference pose, or the base pose of an additive asset). |

## 4. Mirroring rules

- The pose's side comes from the side token in its name.
- Only bones with that side's suffix are mirrored onto their counterparts. Centre bones (no suffix, such as `spine_03` or `head`) and opposite-side bones are never touched.
- Mirror is refused when the name has no single side token, when the pose changes no bone of its side, or when a changed bone has no counterpart in the skeleton.
- An existing counterpart keeps its name, its list position and every bone that isn't mirrored. A new counterpart starts from the base pose.
- Curves of the pose's side (`CR_upperarm_l_up`) copy their value onto the counterpart curve when the asset has it.
- Several poses mirror as one undo step and all or nothing. Selecting a pose together with its own counterpart is refused. **Mirror All** mirrors every authoring-side pose and skips the ones that can't be mirrored.

## 5. Validation

*Validate Assets* on a Pose Asset warns when a right pose no longer mirrors its left pose (mirror the left one again) and when a pose named for one side only moves bones of the other side. A pose without a counterpart is fine.

## 6. Scripting

`UFVPoseAuthoringLibrary` (editor only; each command is one undo step and returns false with an error text when it refuses):

| Function | Does |
|---|---|
| `CreatePose`, `UpdatePose` | Add or replace a pose from a posed Skeletal Mesh Component on the asset's skeleton |
| `RenamePose`, `DuplicatePose`, `DeletePoses` | Name-keyed edits; `DeletePoses` deletes all or none |
| `GetChangedBones` | Bones the pose moves away from the base pose |
| `CanMirrorPose`, `GetMirrorPoseName`, `MirrorPoses`, `MirrorAll` | The mirroring rules above; `MirrorAll` also returns the skipped poses and the orphans, and deletes the orphans on request |
| `CreateInbetweens` | Poses between the base pose and a pose |
| `GetAddableCurves`, `AddCurve`, `SetCurveValue` | Skeleton curves on the asset and their per-pose values |
| `CreatePoseAsset` | An empty, unsaved Pose Asset for a mesh's skeleton, previewed on the mesh |
| `SetPosesFromAnimation` | Frame i of an animation becomes pose i by name: updated (curve values kept) or created, all or nothing. Returns a result struct (created, updated, error) instead of a bool, so Python callers get the error. The Blender tools' **Send Poses to Unreal** uses it |

`UFVAnimationLibrary` (runtime): `GetBoneSide`, `GetMirrorBoneName`, `GetNameSide`, `GetMirrorName`, using the same side pairs.

## 7. Debugging

No debug readout or console commands yet. Automation tests: *Session Frontend > Automation*, filter `FlickerVoid.Animation`. They load the mannequin read-only and work on transient Pose Assets.
