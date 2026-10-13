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
| `DA_Item_<Name>` | `FVItemDefinition` | **Category** (any tag under `Item`), **Weight** (kg per unit), **Grid Size** (footprint, Width x Height cells; each stack claims it once), **Max Quantity** (units per stack; more units start new stacks; 1 = every unit is its own entry, 0 = unlimited), **Quest Item** (cannot be dropped), **Fragments** |

Display info (name, description, icon) comes from the definition's **Display**.

### Fragments

| Fragment | Use for | Fields |
|---|---|---|
| Equippable | Clothes, accessories, outfits | **Slots** (worn in any one of them, e.g. either arm), **Blocked Slots** (covered while worn: what is there comes off and they stay empty), **Granted Tags** (e.g. `Disguise.Faction.Police`), **Equip Conditions** (story gating), **Mesh** (applied by the game layer) |
| Container | Bags and backpacks | **Cells** and **Weight Bonus** added while equipped |
| Usable | Consumables, readables | **Use Conditions**, **Effects**, **Consume On Use** |
| Pickup | Items that can be dropped | **World Mesh** (shown by `AItemPickup` actors holding the item; `Load World Mesh` reads it), **Pickup Class** (spawned on drop; empty uses *Default Pickup Class*) |

Saving *Validate Data* on a definition checks that an Equippable has slots under `Equipment.Slot` and doesn't block its own slots, and warns about a Container that isn't equippable.

### Categories and equipment slots
Only the roots `Item` and `Equipment.Slot` are defined in code; the item and slot pickers show what is under them. The plugin ships example tags in `Plugins/FVInventorySystem/Config/Tags/FVInventoryTags.ini`: categories `Item.Consumable`, `.Quest`, `.Lore`, `.Outfit`, `.Container`, `.Misc` and slots `Equipment.Slot.Head`, `.Eyewear`, `.Underwear.Top`, `.Underwear.Bottom`, `.Socks`, `.UpperBody`, `.LowerBody`, `.Coat`, `.Gloves`, `.Shoes`, `.Accessory.Head/Neck/Arm.Left/Arm.Right/Leg.Left/Leg.Right/Body`, `.Bag`, `.Backpack`.

Add, rename or remove categories and slots in *Project Settings > GameplayTags* like any other tag, under those two roots. Put the game's own tags in the project's `Config/Tags/` and leave the plugin file as the example set.

### Examples
- **Dress**: Slots `Equipment.Slot.UpperBody`, Blocked Slots `Equipment.Slot.LowerBody`.
- **Diving suit**: Slots `Equipment.Slot.UpperBody`, Blocked Slots `.LowerBody`, `.Underwear.Top`, `.Underwear.Bottom`.
- **Bracelet**: Slots `Equipment.Slot.Accessory.Arm.Left` and `.Arm.Right`; it goes on whichever arm is free.

### Example: police uniform
`DA_Item_PoliceJacket`: Category `Item.Outfit`, Weight 1.5, Grid 2x2, Max Quantity 1, Equippable fragment with Slots `Equipment.Slot.Coat` and Granted Tags `Disguise.Faction.Police`. FVSocialSystem then treats the wearer as a police member (see that guide).

## 3. Components

| Component | Goes on | Notes |
|---|---|---|
| FV Inventory Component | Player, containers, NPCs | **Items** = starting stacks (and the saved state). See section 3.1. `Set Bonuses` lets the game layer add allowance (e.g. from an Athletics attribute) |
| FV Equipment Component | Player, NPCs | **Equipped** = starting outfit (and the saved state); starting items are linked to carried stacks at BeginPlay. **Require Inventory** = only carried items can be worn. See section 3.2 |
| FV Item Receiver Component | Anything that can take items | The inventory on the same actor listens to it |
| FV Pickup Component | World items | **Item**, **Quantity**, **Destroy Owner On Pickup**. `Try Pickup(Picker)` hands the item to the picker's receiver; a pickup that doesn't fully fit leaves the rest in the world. `Set Item` changes what it holds (used for dropped items) |

### 3.1 Inventory stacks
- Every entry is a stack with an **Instance Id**, the item, a quantity and optional **Instance Data**. Units of an item without instance data merge into its stacks up to **Max Quantity**, then start new stacks. Each stack claims the item's footprint in cells.
- **Instance data** makes copies of an item differ (dye, wear, a stolen flag, story customization). Derive a struct from `FFVItemInstanceFragment` and mark its fields `SaveGame` so they are saved. Stacks with instance data never merge. `Add Stack` adds one with its data; `Set Instance Data` replaces a stack's data; `Get Stack` reads one by id.
- Item-based calls (`Add Item`, `Remove Item`, `Get Quantity`, `Has Item`, `Use Item`) work across all stacks of the item. `Remove Item` takes from unworn plain stacks first. `Remove Stack` removes from one stack.
- **Dropping**: `Drop Stack(Id, Quantity)` removes units from a stack and spawns them as a pickup in front of the owner (on the ground below the drop point), returning the actor; `Can Drop Stack` tells the UI whether it would work. Quest items can't be dropped, and a worn bag follows the overflow rule below. Instance data is not carried over to the pickup.
- Events: **On Item Changed** (totals of an item), **On Stack Changed** (a stack was added, changed or removed; `Get Stack` fails for a removed one), **On Inventory Reset** (contents replaced by a load; rebuild the UI from `Get Items`), **On Capacity Changed**, **On Item Used**, **On Item Dropped** (item, quantity, spawned pickup).
- Space is a cell count: pockets plus equipped containers plus bonuses. Items have no positions; the UI lists them and shows cells used.

### 3.2 Equipment
- `Equip(Item)` wears a carried, unworn stack in the first of its slots that is free; if none is free it uses the first slot. `Equip Instance(Id, Slot)` wears a specific stack; Slot None picks as above, a named slot moves a worn stack there.
- Whatever conflicts comes off: the item in the chosen slot, items in the new item's Blocked Slots, and items whose Blocked Slots cover the chosen slot. `Get Blocking Item(Slot)` tells the UI why a slot is unavailable.
- **Overflow rule**: capacity never shrinks below what is carried. Taking off or swapping a bag or backpack (`Can Unequip`, `Can Equip`), or dropping a worn one (`Remove Item`/`Remove Stack`), is refused while the remaining space or weight allowance can't hold the carried items. Limits that are switched off don't count. Capacity lowered by `Set Bonuses` isn't checked.
- A worn stack that leaves the inventory is taken off.
- Granted tags are applied to the ASC as loose tags; only the difference is added or removed on each change. Call `Refresh Granted Tags` if the ASC appears after BeginPlay.
- Events: **On Equipment Changed** (slot, old item, new item; one per affected slot, after tags are updated), **On Equipment Reset** (all slots replaced by a load).

## 4. Pickup Blueprints

`BP_Pickup_<Name>` (Actor):
1. A static mesh tagged `Detectable`.
2. **FV Pickup Component** with the item and quantity.
3. **FV Interactable Component** with an interactable definition offering `Interaction.Action.Pickup`, whose effect activates the pickup ability (`Script/Abilities/FVPickupAbility.as` calls `Try Pickup`).

One generic `BP_Pickup` with an editable item works for most items; make specific Blueprints only for special meshes or behaviour.

### Dropped items
Make the generic pickup a Blueprint of **AItemPickup** (AngelScript, `Script/World/Items/ItemPickup.as`: an `AItem` with an FV Pickup Component that shows the item's *Pickup* fragment **World Mesh**), set the interactable definition from step 3 on its Interactable component, and set it as *FlickerVoid > Inventory > Default Pickup Class*. Dropped items spawn as that Blueprint unless their Pickup fragment names another class. Any actor class works as long as it has an FV Pickup Component, including one added in Blueprint.

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
| Default Pickup Class | Actor spawned for dropped items (a Blueprint of `AItemPickup`) |
| Drop Distance | How far in front of the player dropped items land (cm) |

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Inventory.Debug.HUD 1` | Carried items, weight and space on screen |
| `FVCvar.Inventory.EnforceWeight` / `EnforceSpace` | -1 use settings, 0 off, 1 on |
| `FV.Inventory.Give <ItemIdOrAsset> [Quantity]` | Give the player an item |
| `FV.Inventory.Equip <ItemIdOrAsset>` | Equip a carried item |

## 8. UI widgets

C++ widget classes for any item screen (inventory, loot, trade). Make Widget Blueprints of them that add only layout and looks; the screen's graph decides what they show and what clicks do.

| Class | Make | What it does |
|---|---|---|
| `FVItemEntryWidget` | `WBP_ItemEntry` | One stack or an empty cell; no bound widgets. Implement **On Item Changed** (Item, bEmpty), **On Hover Changed** and **On Selected Changed**. Hovered = under the mouse or holding keyboard/gamepad focus; with **Focus On Hover** the mouse moves focus along, so one entry is hovered at a time. A left click or the accept key/button fires **On Clicked**; the entry takes them itself, so its layout holds no Button |
| `FVItemGridWidget` | `WBP_ItemGrid` | Bind **ItemPanel** (Uniform Grid Panel or Wrap Box, in a Scroll Box for long lists). On each placed grid set **Entry Widget Class**, **Cell Count** (cells made up front, also in the designer) and **Columns** (Uniform Grid only). `Set Items` fills the cells in order and adds cells if there are more items. Events: **On Hovered Item Changed**, **On Item Clicked** (a filled cell is selected first), **On Selection Changed**. The selection is kept by instance id across `Set Items`; `Get Focus Target` gives the cell gamepad focus should start on. In a Uniform Grid Panel keys and D-pad step cell by cell, also to cells a Scroll Box has clipped; a cell they focus scrolls into view in the nearest Scroll Box |
| `FVEquipmentSlotWidget` | `WBP_OutfitSlot` | An item entry for one slot: set **Equipment Slot** on each placed one. Shows what the owning player wears there (or the equipment given to `Set Equipment`) and refreshes itself; **On Slot Changed** gives the item covering an empty slot |

Setting them up in the hub's inventory page, with the page's data and action functions, is in the [Game UI guide](GameUI.md#inventory-page).

## Notes

- Inventory and equipment contents, including instance ids and instance data, are saved when the actor also has an **FV Saveable Component** (see FVStorySystem, section 5). After a load both components rebuild capacity and granted tags and broadcast **On Inventory Reset** and **On Equipment Reset**.
- Outfit meshes are not applied by the plugin; bind **On Equipment Changed** and **On Equipment Reset** in the game layer and set the mesh from the Equippable fragment.
