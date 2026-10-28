# Blender and Unreal bridge: plan

Status: plan for review, nothing implemented yet.

## Goal

Blender is the single source of truth for meshes, bones, weights and shape keys. Painting weights and sculpting morphs in UE's Skeletal Mesh Editor is faster to test in game, but those edits then have to come back to the source `.blend` without a manual FBX round trip.

This plan covers two parts of the FlickerVoid Blender extension, the single extension that holds all our Blender tools:

- the **UE link** (remote execution client and vertex matcher), shared by every tool that talks to the editor;
- **Get from Unreal**: copies chosen data from a UE skeletal mesh asset onto the matching source mesh in the open `.blend`. It never writes to UE.

Push to Unreal, LODs, collision and the texture link are other parts of the same extension, planned separately; they reuse the UE link.

The extension is self-contained so it can move to its own repo later: it shares no files or code with the game. It only reads the game project's config ini (contract values such as bone side pairs) and talks to the running editor.

## How it talks to Unreal

Through the engine's Python Remote Execution (Python Editor Script Plugin). Blender finds the editor by UDP multicast on port 6766 and sends Python over a TCP command connection (port 6776), localhost only by default. The client side is the engine's own `remote_execution.py` from the Python Editor Script Plugin, copied into the add-on.

Reference: Epic's [BlenderTools](https://github.com/EpicGames/BlenderTools) and its maintained [poly-hammer fork](https://github.com/poly-hammer/BlenderTools) (both MIT) use the same link in Send to Unreal (`send2ue/dependencies/remote_execution.py`). They also start an XML-RPC server inside the editor (port 9998) to call named functions; we only need a few calls, so plain remote execution is enough, and their RPC layer can be reused if that changes.

No UE plugin and nothing in `Content/`: the UE-side code is sent over the link with each request. The project only needs the Python Editor Script Plugin enabled and Remote Execution on; today neither is set in `FlickerVoid.uproject` or `Config/DefaultEngine.ini`.

## Where it lives

In this repo until it moves to its own; only the parts from this plan are shown.

```
Tools/BlenderBridge/
  Plan.md                    this file
  <extension>/               the one Blender extension (blender_manifest.toml)
    unreal_link/
      remote_execution.py    copied from the engine's Python Editor Script Plugin
      matching.py            vertex matching and weight diff, numpy only, no bpy
    pull/
      unreal_side.py         code sent to the editor, read-only (export only)
      operators.py, ui.py
  tests/                     matching.py tests, plain Python + numpy
```

## Link

The add-on preferences point at the UE project folder. Each Blender mesh object stores the UE asset path in a custom property (`fv_ue_asset`, e.g. `/Game/Characters/Bartender/SK_Bartender`). The panel fills it from the asset selected in the Content Browser.

## Get weights

1. **Export in UE.** `unreal.AssetExportTask` with the FBX exporter, `FbxExportOption.export_source_mesh = True` (source data, so vertices are not split on UV seams), `export_morph_targets = True`, `level_of_detail = False`, into `%TEMP%/FVBridge/`. The editor's in-memory asset is exported, so unsaved paint is included.
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

## Get morphs

From the same FBX: for each chosen morph target, delta = shape key minus basis on the temporary mesh, mapped through the vertex match, written to a shape key of the same name on the source mesh (created, or replaced after a prompt). Shape keys stay relative to Basis, and multires on the base mesh is untouched. This covers the corrective morphs driven by the correctives Control Rig too.

## Out of scope

Push, LODs, collision and textures (other parts of the extension), UVs, normals, animation, live sync, and anything in the pull that writes to UE assets.

## Workflow

Paint or sculpt in UE and test in game, then in Blender press **Get weights** or **Get morphs**, check the report, save the `.blend`, and push again with the extension's Push to Unreal. Pushing before pulling overwrites the UE edits, so the panel warns when the linked asset has unpulled changes (asset dirty or newer than the last pull).

## Phases

The extension grows in small slices that follow the character work (Push and validator, then correctives, one body type, garments). These phases slot into them. Each ends with the owner testing on the real meshes, since cloud sessions can't run Blender with UE.

0. **Setup** (with the Push slice): enable the Python Editor Script Plugin and Remote Execution in the project (config lines only).
1. **UE link and spike** (with the Push slice): connect to the editor, export the source FBX, import it, print vertex count and order checks against the source mesh. Decides index vs KD-tree and FBX vs `SkinWeightModifier`.
2. **Get morphs** (with the correctives slice), so a corrective sculpted in UE ends up in the `.blend`.
3. **Get weights** with both modes and the report (with the garments slice).
4. **Polish**: asset picker, unpulled-changes warning, setup notes.

Estimated size: 600 to 900 lines of Python, most of it in matching and reporting.

## Open questions

1. Manual weight fixes live in separate weight layers. Should a pull write into the final vertex groups, or into a fix layer (for example `FIX_<bone>` groups merged by modifiers)? How are those layers built today?
2. One extension, later in its own repo (decided 2026-10-28). Until then, is `Tools/BlenderBridge/` the right folder, and what should the extension be called?
3. Pull only LOD0 (LODs regenerate from it): OK?
