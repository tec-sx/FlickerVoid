# FlickerVoid Tools for Blender: plan

Status: slice 1 implemented, waiting for the owner's test in Blender and UE; later slices are plans.

## Goal

Blender is the single source of truth for meshes, bones, weights and shape keys. One Blender extension, **FlickerVoid Tools**, covers the whole round trip with Unreal: push, pull, LODs, collision, the texture link, and validators that enforce the character contracts (union skeleton, Mannequin names, `_l`/`_r` sides, max 8 influences, weights from the reference body).

The extension is self-contained so it can move to its own repo later: it shares no files or code with the game. It only reads the game project's config ini (contract values such as bone side pairs) and talks to the running editor.

It grows in small slices that follow the character work. Each slice is usable on its own and adds its checks to the shared validator.

## Slices

| Slice | Tools | Status |
|---|---|---|
| 1 | Core, UE link, **Push to Unreal**, first **Contract Validator** checks | Implemented, to test |
| 2 | **Correctives**: Corrective Pose Exporter (poses and corrective shape keys named for the Meatbag Factory), **Get morphs** from UE | Plan |
| 3 | **One body type**: Body Type Baker for a single extra type, pushed onto the same Skeleton | Plan |
| 4 | **Garments**: Garment Fitter (weights from the reference body incl. shape bones, fitting proxy), **Get weights** from UE | Plan |
| 5 | **LODs and collision**: characters send hand-made LOD0-1 and set UE reduction for LOD2+; props get LODs and collision from Blender (see below) | Plan |
| 6 | **Texture link**: send to Substance Painter, import its exported textures and set up material instances | Plan |
| 7 | As the work needs them: Item and Prop Checker, Hide Mask Baker, Seam Keeper, Face Pose Kit, Deformer Training Data | Ideas |

### LODs: Blender or UE

[blender-lod-generator](https://github.com/dyzlek/blender-lod-generator) (MIT) was reviewed on 2026-10-28. It decimates with Blender's Decimate modifier, guided by a protection mask (UV seams, open borders, materials, silhouette, skinning joints), then repairs UVs from LOD0, transfers LOD0 normals, and can bake normals and AO, make impostors and `UCX_`/`UBX_` colliders, and export an FBX `LodGroup`.

- **Characters stay on UE's reduction.** That add-on applies modifiers, which Blender refuses on meshes with shape keys (it then skips decimation without stopping), and its export leaves out the armature. UE's skeletal reduction keeps skinning and remaps morph targets, removes bones per LOD, and regenerates LODs from every push.
- **Props can use it.** Its seam-aware decimation, UV repair, normal transfer and colliders go further than UE's static mesh reduction for textured props. Push will export a `LodGroup` (LOD children and `UCX_` colliders) so its output, or hand-made LODs, go to UE as they are. Vendoring its core into this extension is an option later; its code and UI are in French.

Not built, because the engine does it: per-LOD bone stripping, animation retargeting, crowd vertex animation (AnimToTexture), physics assets, live sync.

## Layout

```
Tools/BlenderBridge/
  Plan.md
  README.md                    install, UE setup, how to use
  flickervoid_tools/           the Blender extension (blender_manifest.toml)
    core/                      no bpy: contract (ini reader, side names), validation rules
    unreal_link/               no bpy: remote execution client, UE-side snippets
    push/                      Push to Unreal
    validate/                  Contract Validator operators and panel
    preferences.py, scene_data.py, ui.py
  tests/                       plain Python tests for core/ and unreal_link/
```

Code without `bpy` stays in `core/` and `unreal_link/`, so it is tested without Blender.

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

## Get from Unreal (slices 2 and 4)

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
