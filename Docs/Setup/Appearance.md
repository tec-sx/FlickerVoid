# Character appearance (Mutable)

The player wears what the equipment component holds. Garments are fitted in MetaHuman Creator once per body type and exported as skeletal meshes; Mutable merges the worn ones into one **Outfit** mesh at runtime. The player's body is a plain skeletal mesh (the MetaHuman body, exported for UAF) and is never merged, so RigLogic, ML Deformer and Chaos Flesh can run on it later. Skin under clothes is hidden by the body material, not by cutting the body.

NPCs come later: their body goes into the Customizable Object too, with clipping meshes instead of the material mask, and they are generated once at spawn (named NPCs pre-baked with *Bake Instance* if profiling asks for it).

## How it fits together

| Piece | Does |
|---|---|
| *Appearance* item fragment (`FFVItemFragment_Appearance`) | **Meshes**: the garment fitted to each body type (`Appearance.Body.*` tag to skeletal mesh). **Clip Tags**: switched on while the item is worn. |
| *Equippable* fragment's **Mesh** | Fallback for a body type the Appearance fragment doesn't list. |
| `UFVItemLibrary` | `GetAppearanceFragment(Item, Out)` and `LoadAppearanceMesh(Item, BodyType)`; in AngelScript `FVItem::GetAppearanceFragment(...)` and `FVItem::LoadAppearanceMesh(...)`. |
| Tags | Native roots `Appearance.Body` (body types) and `Appearance.Clip` (clip tags). Their children are data you add. |
| `APlayerCharacter` | **Outfit**: skeletal mesh that follows the body's pose (leader pose). **Outfit Customization**: Mutable's Customizable Skeletal Component on it. **Body Type**: the body's `Appearance.Body` tag. |
| `UCharacterAppearance` (`Script/Gameplay/Protagonist/Identity/Appearance`) | At BeginPlay copies the outfit's instance (so play never edits the asset), dresses from the equipment and redresses on every equipment change or reset with one async Mutable update. `SetBodyType(Tag, BodyMesh)` swaps the body and refits every garment. |

**Names are the contract** between the code and your content:

- Each worn slot is a **Skeletal Mesh Parameter named exactly after the slot tag**, e.g. `Equipment.Slot.UpperBody`. Slots the object doesn't have (hands, for example) are skipped.
- Each clip tag is a **bool parameter** in the Customizable Object with the tag's name (tucks now, body clipping meshes for NPCs later) and a **scalar parameter** in the body's materials with the tag's name (1 = hidden). Either one may be missing.

## Editor steps

1. **Plugins**: Mutable, MutableClothing and HairStrandsMutable are already on. Animation runs on UAF as in [Character movement](CharacterMovement.md), step 7.
2. **Tags** (*Project Settings > GameplayTags*): body types under `Appearance.Body` (for example `Appearance.Body.Female.Average.Slim`), clip tags under `Appearance.Clip`, one per garment category or sub-category (for example `Appearance.Clip.UpperBody.LongSleeve`, `Appearance.Clip.Feet.Boots`).
3. **Body material**: for each clip tag the body can hide, a scalar parameter named exactly after the tag, default 0. At 1 it masks that region (opacity mask from a region mask texture), so the material needs a Masked blend mode.
4. **Outfit Customizable Object** (for example `CO_Outfit`):
   - A component named `Outfit`. Its Reference Skeletal Mesh is a body export, for the skeleton and the LODs.
   - One Skeletal Mesh Parameter per worn slot, named after the slot tag, each into its own mesh section. Its default is what an empty slot shows: none, or an empty mesh if Mutable needs one.
   - Tucks between garments (trousers into boots): Clip Mesh With Mesh or Clip With Plane and Morph, switched by bool parameters named after clip tags.
   - Compile options: `CustomizableObjectNumBoneInfluences=Twelve` and Enable 16bit Bone Weight, as Epic advises for MetaHumans.
   - Make an instance (right-click the object > *Create New Instance*), for example `COI_Outfit_Player`.
5. **Player Blueprint** (on `APlayerCharacter`):
   - *Mesh*: the MetaHuman body (UAF export), animated by the UAF System.
   - *Outfit Customization*: *Customizable Object Instance* = `COI_Outfit_Player`, *Component Name* = `Outfit`. The viewport shows the instance's own values.
   - *Body Type*: the body's tag.
   - *FV Equipment Component*: *Equipped* = the starting outfit.
   - The MetaHuman face and grooms stay as the export set them up.
6. **Garments**: on each garment's item definition add the *Appearance* fragment. *Meshes*: one entry per body type with its Creator export. *Clip Tags*: what it covers and tucks. The *Equippable* fragment keeps its Slots, and its Mesh becomes the fallback.
7. **Test**: play, then wear and take off garments in the hub inventory. A garment appears once its Mutable update finishes; the skin under it hides.

## Checks on first run

Written against Epic's 5.8 API pages, not compiled here:

- AngelScript sees Mutable's Blueprint API used by `UCharacterAppearance`: on the instance `Clone`, `ContainsSkeletalMeshParameter`, `SetSkeletalMeshParameterSelectedOption`, `ContainsBoolParameter`, `SetBoolParameterSelectedOption`, `SetDefaultValue`, `UpdateSkeletalMeshAsync`; on the component `GetCustomizableObjectInstance`, `SetCustomizableObjectInstance`.
- The outfit follows the body through leader pose while UAF writes the body's pose (expected, not yet seen).
- `SetDefaultValue` clears a slot whose garment was taken off.
- Modifiers apply to garments set at runtime through Skeletal Mesh Parameters (spike unknown 1 in the Mutable plan).

## Not in this work

NPCs (body in the object, a character appearance fragment, random parameters), the face as a Mutable component, a Wardrobe state for the hub, garments shown in the Blueprint viewport from starting equipment, asynchronous garment loading (they load synchronously now), and `FV.Appearance.*` debug commands.
