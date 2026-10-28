# FVAnimationSystem setup

Left/right naming rules and the Meatbag Factory, the editor tool that authors and mirrors poses in Pose Assets (the target poses read by the corrective rig in the post-process Anim Blueprint). The tool edits stock Pose Assets; it adds no data to them and needs no runtime setup.

Modules: `FVAnimationSystem` (settings, side naming, `UFVAnimationLibrary`), `FVAnimationSystemDebug` (empty for now), `FVAnimationSystemEditor` (pose commands, mirroring, `UFVPoseAuthoringLibrary`). The Meatbag Factory window in the Skeletal Mesh Editor comes next; until then the commands are used from editor scripting.

## 1. Quick start

1. Give the Pose Asset (or its skeleton) a **Preview Mesh**. Mirroring and computed poses are written through it.
2. Name each pose after its side, with a whole side token: `upperarm_l_up_90`, `thigh_r_raise`. A name without one (`ArmRaise`) can't be mirrored.
3. Pose the side bones and add the pose (`CreatePose` from a posed mesh component).
4. Call `MirrorPoses` with the pose names: each counterpart (`upperarm_r_up_90`) is updated if it exists and created if not. Ctrl+Z undoes the whole mirror.

## 2. Settings

*Project Settings > FlickerVoid > Animation System*:

| Setting | Default | Use |
|---|---|---|
| **Side Pairs** | `_l` / `_r` | Bone suffixes that mark a side; matched ignoring case, case kept (`_L` becomes `_R`). Pose and curve names use the same pairs as whole tokens. |
| **Default Mirror Axis** | X | Component-space axis that poses are mirrored across. The Meatbag Factory window can override it for the session (creatures or robots whose pairs mirror on another axis), and scripting passes it per call (`EFVMirrorAxis`). |
| **Changed Rotation / Translation / Scale Tolerance** | 0.01 deg / 0.01 cm / 0.001 | When a bone counts as changed by a pose, compared with the base pose (the reference pose, or the base pose of an additive asset). |

## 3. Mirroring rules

- The pose's side comes from the side token in its name.
- Only bones with that side's suffix are mirrored onto their counterparts. Centre bones (no suffix, such as `spine_03` or `head`) and opposite-side bones are never touched.
- Mirror is refused when the name has no single side token, when the pose changes no bone of its side, or when a changed bone has no counterpart in the skeleton. `CanMirrorPose` returns the reason.
- An existing counterpart keeps its name, its list position and every bone that isn't mirrored. A new counterpart starts from the base pose.
- Curves of the pose's side (`CR_upperarm_l_up`) copy their value onto the counterpart curve when the asset has it.
- Several poses mirror as one undo step and all or nothing. Selecting a pose together with its own counterpart is refused.

## 4. Scripting

`UFVPoseAuthoringLibrary` (editor only; each command is one undo step and returns false with an error text when it refuses):

| Function | Does |
|---|---|
| `CreatePose`, `UpdatePose` | Add or replace a pose from a posed Skeletal Mesh Component on the asset's skeleton |
| `RenamePose`, `DuplicatePose`, `DeletePoses` | Name-keyed edits; `DeletePoses` deletes all or none |
| `GetChangedBones` | Bones the pose moves away from the base pose |
| `CanMirrorPose`, `GetMirrorPoseName`, `MirrorPoses` | The mirroring rules above |

`UFVAnimationLibrary` (runtime): `GetBoneSide`, `GetMirrorBoneName`, `GetNameSide`, `GetMirrorName`, using the same side pairs.

## 5. Debugging

No debug readout or console commands yet. Automation tests: *Session Frontend > Automation*, filter `FlickerVoid.Animation`. They load the mannequin read-only and work on transient Pose Assets.
