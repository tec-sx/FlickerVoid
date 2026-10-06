# FVInventorySystem setup

Items, carrying with weight and space limits, granular outfit slots and world pickups. Outfits grant gameplay tags (disguises, dress codes) that other systems read.

Modules: `FVInventorySystem`, `FVInventorySystemDebug`, `FVInventorySystemEditor`. Uses GameplayAbilities for granted tags.

## 1. Quick start

1. Make an item definition (section 2).
2. Add **FV Inventory Component**, **FV Equipment Component** and **FV Item Receiver Component** to the player.
3. Make a pickup Blueprint (section 4) and place it in the level.
4. Pick it up through an interaction offer (see FVInteractionSystem) or call `Try Pickup`.

## 2. Item definitions

| Asset | Class | Key fields |
|---|---|---|
| `DA_Item_<Name>` | `FVItemDefinition` | **Category** (`Item.Consumable`, `.Quest`, `.Lore`, `.Outfit`, `.Container`, `.Misc`), **Weight** (kg per unit), **Grid Size** (footprint in cells; one stack claims it once), **Max Quantity** (0 = unlimited), **Quest Item** (cannot be dropped), **Fragments** |

Display info (name, description, icon) comes from the definition's **Display**.

### Fragments

| Fragment | Use for | Fields |
|---|---|---|
| Equippable | Clothes, accessories, outfits | **Slot** (`Equipment.Slot.*`), **Granted Tags** (e.g. `Disguise.Faction.Police`), **Equip Conditions** (story gating), **Mesh** (applied by the game layer) |
| Container | Bags and backpacks | **Cells** and **Weight Bonus** added while equipped |
| Usable | Consumables, readables | **Use Conditions**, **Effects**, **Consume On Use** |

### Equipment slots
Defined natively: `Equipment.Slot.Head`, `.Eyewear`, `.Underwear.Top`, `.Underwear.Bottom`, `.Socks`, `.UpperBody`, `.LowerBody`, `.Coat`, `.Gloves`, `.Shoes`, `.Accessory.Head/Neck/Arm.Left/Arm.Right/Leg.Left/Leg.Right/Body`, `.Bag`, `.Backpack`. Item categories `Item.*` are native too.

### Example: police uniform
`DA_Item_PoliceJacket`: Category `Item.Outfit`, Weight 1.5, Grid 2x2, Equippable fragment with Slot `Equipment.Slot.Coat` and Granted Tags `Disguise.Faction.Police`. FVSocialSystem then treats the wearer as a police member (see that guide).

## 3. Components

| Component | Goes on | Notes |
|---|---|---|
| FV Inventory Component | Player, containers, NPCs | **Items** = starting contents (and the saved state). `Add Item` returns how many fit; `Use Item`, `Get Items In Category`, weight and cell queries; events **On Item Changed**, **On Capacity Changed**, **On Item Used**. `Set Bonuses` lets the game layer add allowance (e.g. from an Athletics attribute) |
| FV Equipment Component | Player, NPCs | **Equipped** = starting outfit (and the saved state). **Require Inventory** = only carried items can be worn. Worn items' granted tags go onto the owner's Ability System Component as loose tags; call `Refresh Granted Tags` if the ASC appears after BeginPlay |
| FV Item Receiver Component | Anything that can take items | The inventory on the same actor listens to it |
| FV Pickup Component | World items | **Item**, **Quantity**, **Destroy Owner On Pickup**. `Try Pickup(Picker)` hands the item to the picker's receiver; a pickup that doesn't fully fit leaves the rest in the world |

## 4. Pickup Blueprints

`BP_Pickup_<Name>` (Actor):
1. A static mesh tagged `Detectable`.
2. **FV Pickup Component** with the item and quantity.
3. **FV Interactable Component** with an interactable definition offering `Interaction.Action.Pickup`, whose effect activates the pickup ability (`Script/Abilities/FVPickupAbility.as` calls `Try Pickup`).

One generic `BP_Pickup` with an editable item works for most items; make specific Blueprints only for special meshes or behaviour.

## 5. Conditions and effects

| Conditions | Effects |
|---|---|
| Has Item (quantity; Instigator or Target), Is Wearing (an item, or anything granting a tag) | Give Item (optionally equip), Take Item |

Use *Is Wearing* with a granted tag for dress codes ("wear anything granting `Disguise.Faction.Police`").

## 6. Settings

*FlickerVoid > Inventory*:

| Setting | Meaning |
|---|---|
| Enforce Weight / Enforce Space | Switch each limit on or off |
| Base Weight Limit | kg carried with nothing equipped |
| Base Pocket Cells | Cells available without a bag |
| Grid Width | Inventory grid width; footprints are measured against it |

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Inventory.Debug.HUD 1` | Carried items, weight and space on screen |
| `FVCvar.Inventory.EnforceWeight` / `EnforceSpace` | -1 use settings, 0 off, 1 on |
| `FV.Inventory.Give <ItemIdOrAsset> [Quantity]` | Give the player an item |
| `FV.Inventory.Equip <ItemIdOrAsset>` | Equip a carried item |

## Notes

- Inventory and equipment contents are `SaveGame` properties; they are written to a save only when a save participant does it (see FVStorySystem, section 5).
- Outfit meshes are not applied by the plugin; bind **On Equipment Changed** in the game layer and set the mesh from the Equippable fragment.
