# FlickerVoid Tools for Blender: plan

Status: slice 1 and the Meatbag Factory correctives are implemented and waiting for the owner's test in Blender and UE; later slices are plans.

## Goal

Blender is the single source of truth for meshes, bones, weights and shape keys. One Blender extension, **FlickerVoid Tools**, covers the whole round trip with Unreal: push, pull, LODs, collision, the texture link, and validators that enforce the character contracts (union skeleton, Mannequin names, `_l`/`_r` sides, max 8 influences, weights from the reference body).

The extension is self-contained so it can move to its own repo later: it shares no files or code with the game. It only reads the game project's config ini (contract values such as bone side pairs) and talks to the running editor.

Everything that makes characters (contract checks, correctives, body types, garments) lives under **Meatbag Factory**, the name the project uses for character creation, in Blender and in Unreal.

It grows in small slices that follow the character work. Each slice is usable on its own and adds its checks to the shared validator.

## Slices

| Slice | Tools | Status |
|---|---|---|
| 1 | Core, UE link, **Push to Unreal**, first **Contract** checks | Implemented, to test |
| 2 | **Correctives**: helper bone solver (port of Dem Bones), residual `CR_` shape keys, **Send Poses to Unreal** into a Pose Asset | Implemented, to test (needs PR #66) |
| 2b | **Get morphs** from UE | Plan |
| 3 | **One body type**: Body Type Baker for a single extra type, pushed onto the same Skeleton | Plan |
| 4 | **Garments**: Garment Fitter (weights from the reference body incl. shape bones, fitting proxy), **Get weights** from UE | Plan |
| 5 | **LODs and collision**: characters send hand-made LOD0-1 and set UE reduction for LOD2+; static meshes get LODs and impostors from the Static Mesh LOD tool and colliders from the Collision tool (see below) | Plan |
| 6 | **Texture link**: send to Substance Painter, import its exported textures and set up material instances | Plan |
| 7 | As the work needs them: Item and Prop Checker, Hide Mask Baker, Seam Keeper, Face Pose Kit, Deformer Training Data | Ideas |

### LODs: Blender or UE

[blender-lod-generator](https://github.com/dyzlek/blender-lod-generator) (MIT) was reviewed on 2026-10-28. It decimates with Blender's Decimate modifier, guided by a protection mask (UV seams, open borders, materials, silhouette, skinning joints), then repairs UVs from LOD0, transfers LOD0 normals, and can bake normals and AO, make impostors and `UCX_`/`UBX_` colliders, and export an FBX `LodGroup`.

- **Characters stay on UE's reduction.** That add-on applies modifiers, which Blender refuses on meshes with shape keys (it then skips decimation without stopping), and its export leaves out the armature. UE's skeletal reduction keeps skinning and remaps morph targets, removes bones per LOD, and regenerates LODs from every push.
- **Static meshes use it, inside this extension** (decided 2026-10-28). Its core moves in as the Static Mesh LOD tool: seam-aware decimation, UV repair from LOD0, normal transfer, culling of small parts, error-budget and screen-size modes, normal and AO bakes and impostors; collision comes from the collision tool below. It is translated to English, keeps its MIT notice, and is refused on meshes with shape keys or an armature. Push exports the result as a `LodGroup` (LOD children and colliders), so UE imports the LODs as they are.

### Collision

Our own collision tool for static meshes, replacing the LOD tool's single-hull colliders:

- **Shapes**: box (axis-aligned or PCA-oriented, whichever is tighter), sphere, capsule along the longest axis, single convex hull, and multi-hull convex decomposition with V-HACD (BSD-3) for concave props. Named `UBX_`, `USP_`, `UCP_`, `UCX_` with `_01`, `_02` per part and `_LODn` stripped from the source name. No decimated mesh named `UCX_`, since UE hulls every `UCX_` part.
- **Fit**: hulls snapped back to the surface after decomposition, a padding offset for every shape, origin, rotation and non-uniform scale handled.
- **Workflow**: a suggestion button that picks the shape and hull count from the mesh (convexity, fill, roundness, loose parts), settings stored on each collider so Regenerate refits with its own settings, scope (selected, collection, scene), and a report of missing colliders, naming drift, unapplied scale and heavy hulls.
- **Push** sends colliders with their mesh in the same FBX.

Not built, because the engine does it: per-LOD bone stripping, animation retargeting, crowd vertex animation (AnimToTexture), physics assets, live sync.

## Layout

```
Tools/BlenderBridge/
  Plan.md
  README.md                    install, UE setup, how to use
  flickervoid_tools/           the Blender extension (blender_manifest.toml)
    core/                      no bpy: contract (ini reader, side names), validation rules
    unreal_link/               no bpy: remote execution client, UE-side snippets
    push/                      Push to Unreal, FBX export
    meatbag_factory/           character creation
      contract/                Contract checks, operators
      solver.py                no bpy: helper bone solver (SSDR, port of Dem Bones)
      capture.py, apply.py     read the problem from the scene, write the solve back
      library.py               the pose library (hidden action) and preview blending
      sculpt.py                the managed sculpt target
      muscles.py, spindle.py   muscles that push the sculpt target's skin
      pose_asset.py            bake the library poses into an FBX animation
    preferences.py, scene_data.py, ui.py
  tests/                       plain Python tests for core/ and unreal_link/
```

Code without `bpy` stays in `core/`, `unreal_link/` and `meatbag_factory/solver.py`, so it is tested without Blender.

## Contract

The add-on preferences point at the UE project folder. From its `Config/DefaultGame.ini` the extension reads the Animation System settings (`[/Script/FVAnimationSystem.FVAnimationSettings]`, `SidePairs`), the same values the Meatbag Factory uses, with the engine defaults (`_l`/`_r`) when the section is missing. Rules UE has no setting for live in the add-on preferences: bone budget (300), max influences (8), required bone names (the Mannequin body chain) and the expected scene unit scale (0.01, centimetres).

## UE link

Through the engine's Python Remote Execution (Python Editor Script Plugin). Blender finds the editor by UDP multicast (`239.0.0.1:6766`), opens a TCP command server (`127.0.0.1:6776`) that the editor connects to, and sends Python to run there. The client is our own small implementation of the engine's protocol (`PythonScriptRemoteExecution.cpp`, mirrored by the engine's `remote_execution.py`); Epic's MIT-licensed [BlenderTools](https://github.com/EpicGames/BlenderTools) and its maintained [poly-hammer fork](https://github.com/poly-hammer/BlenderTools) use the same link in Send to Unreal and are the reference. Results come back as one JSON line printed by the UE-side snippet.

No UE plugin and nothing in `Content/`: the UE-side code is sent with each request. The project only needs the Python Editor Script Plugin with Remote Execution on (set in `FlickerVoid.uproject` and `Config/DefaultEngine.ini` by slice 1).

## Slice 1: Push to Unreal

- **Link.** Each mesh or armature object stores its UE asset path in the custom property `fv_ue_asset` (for example `/Game/Characters/Bartender/SK_Bartender`) and, for skeletal meshes, the Skeleton in `fv_ue_skeleton`. **Use Selected Asset** fills them from the asset selected in the Content Browser.
- **Export.** One locked FBX preset: selection only, armature and meshes, deform bones only, no leaf bones, `Y`/`X` bone axes, unit scale applied, tangent space, smoothing by face, shape keys as morph targets, no animation. The armature object is exported as `Armature` when its top bone is `root`, so UE doesn't add an extra root bone. Files go to a temp folder.
- **Import.** In UE an `AssetImportTask` imports or reimports onto the linked path: skeletal mesh on the linked Skeleton, morph targets on, normals and tangents imported, no materials, textures or animations, existing asset replaced. The asset is not saved; you save in UE.
- **Guard.** Push runs the validator first and stops on errors (warnings are listed and pushed anyway).

## Slice 1: Contract Validator

Checks the selected armatures and meshes and lists problems in the panel; select a row to select the bone or vertices.

| Check | Level |
|---|---|
| Scene unit scale is the contract's (0.01) | Error |
| Every `_l` bone has a `_r` counterpart and the other way round (side pairs from the ini) | Error |
| Required body-chain bones exist (Mannequin names) | Error |
| Bone count within the budget | Error |
| Armature has one top bone | Error |
| Vertex groups without a matching deform bone | Warning |
| Vertices with more influences than the limit | Warning, fixable |
| Vertices whose weights don't sum to 1 | Warning, fixable |
| Vertices with no weight | Error |
| Mesh has no Armature modifier pointing at the armature | Error |

Fixes are only the safe ones (limit influences, normalize), each one undo step. Renames stay manual.

Later slices add rest-pose symmetry, bone roll and axis checks, and the leaf rule for extra bones.

## Slice 2: Meatbag Factory correctives

Turns a target shape (a muscle simulation, a sculpt) into helper bone motion, so correctives are bone driven where bones can do it and morphs only carry the rest.

### Pose library

The example poses live in a **pose library**, not on the timeline. The library is one action per armature that is never assigned to it, so the timeline, the Dope Sheet and the armature's own animation stay untouched. Pose i is keyed at frame i on every bone (location, quaternion rotation, scale, relative to rest), and the pose names and preview weights are stored on the action, so the action is the whole library and saves with the `.blend`. The Pose Asset gets the poses in library order.

- **Add Pose** (+) adds the viewport pose after the active one; **Add Rest Pose** adds one with every bone transform cleared, to pose from scratch.
- **Update Pose** replaces the active pose with the viewport pose. **Remove**, **Move Up/Down** and renaming in the list rewrite the action's keys to match. Names stay unique.
- The bone icon on a row shows that pose. **Rest Pose** clears the bones and the weights.
- **Preview weights**: each pose has a weight slider and the viewport shows the mix. **Blend** is a weighted average where the rest pose takes what is left below 1, so A at `1 - t` and B at `t` interpolates between two poses, and a single pose at `t` goes from rest to the pose. **Additive** adds each pose's change from rest on top of the others, the way stacked correctives combine.

Considered and not taken: Blender's own Pose Library (one action asset per pose in the Asset Browser, with interactive blending) has no order and scatters the poses over many assets; timeline markers tie the poses to the scene's frames. Muscle simulations that need frames to settle are the one case the library can't drive yet; a **Lay Out on Timeline** command (poses on temporary frames, simulated, target read per frame, removed afterwards) would cover it when needed.

### Setup in Blender

- **Armature** with its example poses in the pose library, and helper bones (deform bones, usually parented to a main bone) in a bone collection, `Correctives` by default.
- **Mesh**: the skinned body. Helpers need painted weights on the vertices they should move; a helper without weights can't be solved and is reported.
- **Target**: the shape to match in each pose. By default the **sculpt target**, which the tool makes and manages (below). **Other Target** picks any mesh with the same vertices instead, such as a muscle skin driven by the same bones. In either, shape keys named after a pose are set to 1 for that pose and 0 for the others while capturing. Its modifiers may move vertices but not change their count.
- Pose names become the Pose Asset's pose names and the `CR_` shape key names, so they use the side tokens the Meatbag Factory mirrors (`thigh_l_raise_90`).
- The Armature modifier has Preserve Volume off, since Unreal skins linearly.

### Sculpt target

You pose only one armature. The sculpt target is a hidden copy of the body (`<Body>_MF_Target`) on the same armature, made on first use, with its own Basis and one shape key per library pose. The body stays clean: it is never sculpted, and the target is left out of Push and the Contract checks.

- **Sculpt Pose** shows the active library pose, hides the body, shows the target with that pose's key active (made when missing, shown alone) and enters Sculpt mode. Blender maps the strokes on the posed mesh back into the key.
- **Back** returns to the body and, with **Solve on Back**, solves that pose right away.
- Renaming or removing a pose renames or removes its key. When the body's vertex count changes, delete the target and a new one is made (sculpts on the old one are lost).

### Muscles

Muscles shape the sculpt target the way a muscle simulation would, but follow each pose at once (no frames, no simulation), so the pose library and the solve drive them directly. Jiggle is left to Unreal at runtime.

- **Add Muscle** between two bones (in Pose mode, select the origin bone, then the insertion bone as active). It makes an origin anchor and an insertion anchor (empties on the bones, at a fraction along each) and a spindle mesh between them, sized to sit inside the skin. Move the anchors to place it.
- The spindle is our own procedural mesh: **Width** and **Thickness** of the belly, **Belly Position**, **Tendon** radius at the ends. It is parented to the origin anchor and stretched to the insertion anchor with Blender's Stretch To constraint keeping volume, so it thickens as it shortens (**Volume** scales that).
- On the sculpt target each muscle adds a Shrinkwrap (skin inside the muscle moves to its surface, plus **Skin Offset**), masked by the two bones' weights faded out past the muscle's ends, and one Corrective Smooth after all muscles cleans up the pushed skin.
- **Mirror Muscle** makes or replaces the other side's muscle from the contract's side names, mirrored across the armature's X. **Preview Skin** swaps the body and the target in the viewport.
- Sculpt keys on the target still apply under the muscles, for shapes muscles can't make.
- **Generate Muscles** builds the human template (`anatomy.py`) on an Unreal Mannequin skeleton (UE4 or UE5 bone names; `chest` and `midspine` resolve to the bones that exist), replacing earlier generated muscles and keeping hand-added ones. Every muscle is written for the left side and mirrored. An attachment is a bone, a fraction along it, a direction in the body's frame (front, left, up, from the skeleton) and a depth: the point is cast toward the skin in that direction and kept that far back under it, so placement fits any body. Each strand is sized to its clearance from the skin, then **Fit to Skin** casts a ray outward from the muscle's line through every ring vertex and gives it 85% of the room it finds (belly fills it, tendons keep their taper; growth capped so a muscle doesn't fill the body cavity), so muscles flatten under flat skin and fill round limbs, and the skin doesn't move at rest. The fit is kept per muscle and survives shape edits; run Fit to Skin again after moving anchors or changing the body.
- A muscle with several attachments (pectoral, deltoid, trapezius, latissimus, quadriceps) is a **fan** of strands sharing a group name and a colour (`Pectoral1_l`..`Pectoral3_l`). A broad shape several joints bend (gluteus) is a **pad**: a flat strand that also follows extra bones through Hook modifiers with a smooth falloff.
- View: per-muscle material colour with **Opacity** (Solid shading, Color: Material), Solid/Wire, **In Front** and show/hide in the Muscles panel.
- Later, **muscle-driven solve**: helper bones fitted to the muscle surfaces instead of the skin (about one helper per muscle group).
- Later, **fat**: a painted `MF_Fat` thickness map that raises the skin off the muscles (with a Fat Amount slider), pose-driven sag along gravity and squash at folding joints, and soft pads for belly, breasts and buttocks whose helpers can jiggle in Unreal at runtime.

### Solve scope

- **Solve**: **All Poses** (every pose that isn't locked) or **Active Pose**.
- **Selected Helpers Only**: only the selected helper bones are solved; the others keep what the pose gives them.
- **Lock** a pose in the list after tuning its helpers by hand, so later solves leave it alone. Without a lock, a solve overwrites the helpers of every pose it covers.
- **Solve Weights** needs every pose, so it is skipped (and reported) when the scope leaves poses out.

### Solve Helper Bones

A numpy port of the parts of [Dem Bones](https://github.com/electronicarts/dem-bones) (BSD-3) that apply: Smooth Skinning Decomposition with Rigid Bones, with the main bones locked and only the helpers solved.

1. **Capture**: for each library pose, the main bones' posed matrices with the helpers at rest, and the target's vertex positions, all in armature space.
2. **Transforms**: Gauss-Seidel sweeps over the helpers. Each helper's rotation and translation (and uniform scale with **Allow Scale**) is the weighted Procrustes fit that brings its vertices closest to the target, with the other bones fixed.
3. **Weights** (optional, **Solve Weights**): per vertex, non-negative least squares on how much it follows each helper it already has; the main bones keep their painted proportions in what is left, and the vertex keeps at most the contract's influence count. Alternates with step 2 for **Weight Rounds**.
4. **Write back**: each helper's solved transform stored in its library pose, changed weights written, the largest remaining error per vertex in the point attribute `mf_residual` (view it as a color attribute), and, with **Residual Shape Keys**, a `CR_<pose>` shape key per pose whose error exceeds the **Threshold**. A residual key is stored in rest space, so the solved bones plus the key at 1 give the target exactly.

The panel lists each pose's RMS error before and after. Helpers are never driven here: the corrective rig in Unreal drives them from the Pose Asset.

### Send Poses to Unreal

1. Each library pose (every bone, after constraints and drivers) is baked into frame i of a temporary armature copy without constraints, exported as an armature-only FBX animation with the push preset.
2. In Unreal, the snippet imports it as `<Pose Asset>_Source` on the mesh's Skeleton, creates the Pose Asset if it is missing (`CreatePoseAsset`, previewed on the linked mesh), and calls `SetPosesFromAnimation`: poses with a library pose's name are updated (keeping their curve values), the others created, as one undo step.
3. The source animation is deleted unless **Keep Source Animation** is on. Nothing is saved.

Push the mesh first, so the Skeleton has the helper bones and the `CR_` morph targets. Right-side poses come from **Mirror All** in the Meatbag Factory window.

## Get from Unreal (slices 2b and 4)

Copies chosen data from a UE skeletal mesh asset onto the matching source mesh in the open `.blend`. It never writes to UE.

### Get weights

1. **Export in UE.** `unreal.AssetExportTask` with the FBX exporter, `FbxExportOption.export_source_mesh = True` (source data, so vertices are not split on UV seams), `export_morph_targets = True`, `level_of_detail = False`, into a temp folder. The editor's in-memory asset is exported, so unsaved paint is included.
2. **Import in Blender** into a temporary hidden collection with the same axis and scale settings as the push, then delete it at the end.
3. **Match vertices** against the source mesh's base level (no modifiers, multires level 0, rest pose):
   - same vertex count and every position within tolerance: match by index;
   - otherwise: KD-tree nearest position (`mathutils.kdtree`) within tolerance; abort with a report if too many vertices are unmatched or two map to one.
4. **Diff.** Compare UE weights with the Blender weights after simulating UE's own pruning (top 8 influences, normalized), so the 8-influence limit doesn't show up as a change everywhere.
5. **Apply**, one of two modes:
   - **Changed vertices** (default): every vertex whose weights changed gets its full UE influence set. Keeps UE's normalization exactly as painted.
   - **Chosen bones**: copy only the chosen bones (selected in pose mode or picked from a list of bones that changed), then renormalize the other influences around them, like Blender's normalize with locked groups.
6. **Report**: vertices changed per bone, unmatched vertices, one undo step for the whole pull.

Optional, decided in the spike: read weights directly with `unreal.SkinWeightModifier` (`get_vertex_weights` per vertex) instead of from the FBX. It skips the FBX round trip for weights, but needs its vertex IDs to match Blender's order.

### Get morphs

From the same FBX: for each chosen morph target, delta = shape key minus basis on the temporary mesh, mapped through the vertex match, written to a shape key of the same name on the source mesh (created, or replaced after a prompt). Shape keys stay relative to Basis, and multires on the base mesh is untouched. This covers the corrective morphs driven by the correctives Control Rig too.

### Workflow

Paint or sculpt in UE and test in game, then in Blender press **Get weights** or **Get morphs**, check the report, save the `.blend`, and push again. Pushing before pulling overwrites the UE edits, so the panel will warn when the linked asset has unpulled changes (asset dirty or newer than the last pull).

## Testing

Cloud sessions can't run Blender or UE, so every slice ends with the owner testing on real files. `tests/` runs the bpy-free parts with plain Python: `python -m unittest discover Tools/BlenderBridge/tests`.

## Open questions

1. Manual weight fixes live in separate weight layers. Should Get weights write into the final vertex groups, or into a fix layer (for example `FIX_<bone>` groups merged by modifiers)? How are those layers built today?
2. The extension is called **FlickerVoid Tools** (`flickervoid_tools`) and lives in `Tools/BlenderBridge/` until it moves to its own repo. OK?
3. Pull only LOD0 (LODs regenerate from it): OK?
4. UE 5.8 may route FBX through Interchange, which ignores the legacy `FbxImportUI` options. The slice 1 test shows whether the push needs an Interchange pipeline instead.
