# Character appearance (Mutable)

One Mutable instance builds the player from two components of one Customizable Object: **Body**, the body type with the skin under worn garments cut away by clipping meshes, and **Outfit**, the worn garments merged into one mesh. Garments are fitted in MetaHuman Creator once per body type and exported as skeletal meshes. The body stays its own mesh, never merged with the garments.

NPCs come later: body and garments in one component, generated once at spawn (named NPCs pre-baked with *Bake Instance* if profiling asks for it).

## How it fits together

| Piece | Does |
|---|---|
| *Appearance* item fragment (`FFVItemFragment_Appearance`) | **Meshes**: the garment fitted to each body type (`Appearance.Body.*` tag to skeletal mesh). **Clip Tags**: switched on while the item is worn. |
| *Equippable* fragment's **Mesh** | Used for a body type the Appearance fragment doesn't list, so a garment that fits every body needs no per-body meshes. |
| `UFVItemLibrary` | `GetAppearanceFragment(Item, Out)` and `LoadAppearanceMesh(Item, BodyType)`; in AngelScript `FVItem::GetAppearanceFragment(...)` and `FVItem::LoadAppearanceMesh(...)`. |
| Tags | Native roots `Appearance.Body` (body types) and `Appearance.Clip` (clip tags). Their children are data you add. |
| `APlayerCharacter` | **Body Customization**: Mutable's Customizable Skeletal Component on the body (*Mesh*). **Outfit**: skeletal mesh that follows the body's pose (leader pose). **Outfit Customization**: the Customizable Skeletal Component on it. **Body Type**: the body's `Appearance.Body` tag. |
| `UCharacterAppearance` (`Script/Gameplay/Protagonist/Identity/Appearance`) | At BeginPlay copies the body's instance and gives the copy to both components (so play never edits the asset), then dresses from the equipment and redresses on every equipment change or reset with one async Mutable update. `SetBodyType(Tag)` switches the body and refits every garment. |

**Names are the contract** between the code and your content:

- The body types are an enum parameter named `Appearance.Body` whose options are named exactly after the body type tags, e.g. `Appearance.Body.Female.Average.Slim`.
- Each worn slot is a **Skeletal Mesh Parameter named exactly after the slot tag**, e.g. `Equipment.Slot.UpperBody`. Slots the object doesn't have (hands, for example) are skipped.
- Each clip tag is a **toggle (bool parameter) named exactly after the tag**, e.g. `Appearance.Clip.UpperBody.LongSleeve`. What it holds (clipping meshes on the body, tucks between garments) is on while a worn garment has the tag. A clip tag without a toggle does nothing.

## Editor steps

1. **Plugins**: Mutable, MutableClothing and HairStrandsMutable are already on. Animation runs on UAF as in [Character movement](CharacterMovement.md), step 7.
2. **Tags** (*Project Settings > GameplayTags*): body types under `Appearance.Body` (for example `Appearance.Body.Female.Average.Slim`), clip tags under `Appearance.Clip`, one per garment category or sub-category (for example `Appearance.Clip.UpperBody.LongSleeve`, `Appearance.Clip.Feet.Boots`).
3. **Character Customizable Object** (for example `CO_Character`):
   - Two components, `Body` and `Outfit`. Each Reference Skeletal Mesh is a body export, for the skeleton and the LODs.
   - **Body types**: an Object Group with Group Name `Appearance.Body`, type *At least one Option*, with one child object per body type whose Object Name is its tag. Each child adds its body export to the `Body` component through an ordinary Skeletal Mesh node, in a mesh section tagged with the body type tag and its parents (`Appearance.Body.Female`, ...), so clipping can aim at one body type or a whole group.
   - **Garments**: in the `Outfit` component, one Skeletal Mesh Parameter per worn slot, named after the slot tag, each into its own mesh section tagged with the slot tag. Its default is what an empty slot shows: none, or an empty mesh if Mutable needs one.
   - **Clipping**: an Object Group of type *Toggle* with one child object per clip tag whose Object Name is the tag. Each child holds a Clip Mesh With Mesh per body type (or one roomy clipping mesh per gender), with Required Tags set to the body sections it cuts. Tucks between garments (trousers into boots) go in the same child: Clip Mesh With Mesh, or Clip With Plane and Morph, aimed at the garment's slot tag.
   - Compile options: `CustomizableObjectNumBoneInfluences=Twelve` and Enable 16bit Bone Weight, as Epic advises for MetaHumans.
   - Make an instance (right-click the object > *Create New Instance*), for example `COI_Character_Player`.
4. **Player Blueprint** (on `APlayerCharacter`):
   - *Mesh*: a MetaHuman body export (UAF), animated by the UAF System. Mutable replaces it with the body it builds.
   - *Body Customization*: *Customizable Object Instance* = `COI_Character_Player`, *Component Name* = `Body`.
   - *Outfit Customization*: the same instance, *Component Name* = `Outfit`. The viewport shows the instance's own values.
   - *Body Type*: the body's tag.
   - *FV Equipment Component*: *Equipped* = the starting outfit.
   - The MetaHuman face and grooms stay as the export set them up.
5. **Garments**: on each garment's item definition add the *Appearance* fragment. *Meshes*: one entry per body type with its Creator export. *Clip Tags*: what it covers and tucks. The *Equippable* fragment keeps its Slots, and its Mesh serves any body type *Meshes* doesn't list.
6. **Test**: play, then wear and take off garments in the hub inventory. Once a garment's Mutable update finishes it appears and the skin under it is cut away.

## Checks on first run

Written against Epic's 5.8 API pages, not compiled here:

- AngelScript sees Mutable's Blueprint API used by `UCharacterAppearance`: on the instance `Clone`, `ContainsEnumParameter`, `SetEnumParameterSelectedOption`, `ContainsSkeletalMeshParameter`, `SetSkeletalMeshParameterSelectedOption`, `ContainsBoolParameter`, `SetBoolParameterSelectedOption`, `SetDefaultValue`, `UpdateSkeletalMeshAsync`; on the component `GetCustomizableObjectInstance`, `SetCustomizableObjectInstance`.
- UAF keeps animating the body once Mutable gives *Mesh* the body it built, and the outfit follows through leader pose (expected, not yet seen).
- What the MetaHuman body export carries for its rig (asset user data such as DNA) survives on the built body (spike unknown 2 in the Mutable plan).
- `SetDefaultValue` clears a slot whose garment was taken off.
- Tucks work on garments set through Skeletal Mesh Parameters (spike unknown 1). [A forum report](https://forums.unrealengine.com/t/mutable-crash-when-combining-skeletal-mesh-parameters-with-clip-mesh-modifiers/2687122) crashed when clip modifiers cut a mesh parameter, and Epic answered that 5.7's mesh parameters were still early. The body uses ordinary mesh nodes, which that report says work.

## Not in this work

NPCs (body and garments in one component, a character appearance fragment, random parameters), the face as a Mutable component, a Wardrobe state for the hub, garments shown in the Blueprint viewport from starting equipment, asynchronous garment loading (they load synchronously now), ML Deformer and Chaos Flesh on the hero (both bind to one fixed mesh and the built body changes with the outfit; inferred, not tested), and `FV.Appearance.*` debug commands.
