# FVInventorySystem setup

Items, carrying with weight and space limits, a separate Keys and Quest list, money, granular outfit slots, world pickups and loot tables. Outfits grant gameplay tags (disguises, dress codes) that other systems read.

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

Display info (name, description, icon) comes from the definition's **Display**. The definition's **Tags** (from `UFVDefinition`) carry what the item *is* for other systems: a key tag under `Key` (`Key.Door.Mansion`) that locks match, or a tool tag under `Tool` (`Tool.Lockpick`, `Tool.Credential.Police`). `Find Item With Tag` and the *Has Item With Tag* condition read them.

`Is Key Item` is true when the category is under one of the settings' **Key Item Categories** (section 7; `Item.Key` and `Item.Quest` by default). Key items are listed apart (section 3.4) and take no weight and no cells.

### Fragments

| Fragment | Use for | Fields |
|---|---|---|
| Equippable | Clothes, accessories, outfits, items held in a hand | **Slots** (worn in any one of them, e.g. either arm), **Blocked Slots** (covered while worn: what is there comes off and they stay empty), **Granted Tags** (e.g. `Disguise.Faction.Police`), **Equip Conditions** (story gating), **Mesh** (applied by the game layer) |
| Container | Bags and backpacks | **Cells** and **Weight Bonus** added while equipped |
| Usable | Consumables, sobering items, medicine | **Use Conditions**, **Effects** (applied with the user as Instigator and Target), **Consume On Use** |
| Charges | Several uses per unit, e.g. sips in a bottle | **Charges** (uses per unit). The unit is used up after its last use, whatever Consume On Use says |
| Value | Anything that can be traded | **Base Price** (one unit, before the merchant's multipliers), **Sellable** |

Consumables stay generic: what a drink, food or medicine does is its Usable effects (e.g. *Modify Attribute* on an Intoxication or Health attribute). There is no energy system.

Saving *Validate Data* on a definition checks that an Equippable has slots under `Equipment.Slot` and doesn't block its own slots, and warns about a Container that isn't equippable and about Charges without a Usable fragment.

### Categories and equipment slots
Only the roots `Item` and `Equipment.Slot` are defined in code; the item and slot pickers show what is under them. The plugin ships example tags in `Plugins/FVInventorySystem/Config/Tags/FVInventoryTags.ini`:
- Categories `Item.Consumable` (`.Drink`, `.Food`, `.Medicine`), `.Quest`, `.Key` (keys and keycards), `.Lore`, `.Outfit`, `.Container`, `.Valuable` (sellable goods), `.Held` (carried in a hand), `.Throwable`, `.Misc`.
- Slots `Equipment.Slot.Head`, `.Eyewear`, `.Underwear.Top`, `.Underwear.Bottom`, `.Socks`, `.UpperBody`, `.LowerBody`, `.Coat`, `.Gloves`, `.Shoes`, `.Accessory.Head/Neck/Arm.Left/Arm.Right/Leg.Left/Leg.Right/Body`, `.Bag`, `.Backpack`, `.Hand.Left`, `.Hand.Right`.
- Roots `Key` (key tags locks match; the keys themselves are your data, e.g. `Key.Door.Mansion`) and `Tool` (tool and credential tags, e.g. `Tool.Lockpick`).

Add, rename or remove categories and slots in *Project Settings > GameplayTags* like any other tag, under those roots. Put the game's own tags in the project's `Config/Tags/` and leave the plugin file as the example set.

### Examples
- **Dress**: Slots `Equipment.Slot.UpperBody`, Blocked Slots `Equipment.Slot.LowerBody`.
- **Diving suit**: Slots `Equipment.Slot.UpperBody`, Blocked Slots `.LowerBody`, `.Underwear.Top`, `.Underwear.Bottom`.
- **Bracelet**: Slots `Equipment.Slot.Accessory.Arm.Left` and `.Arm.Right`; it goes on whichever arm is free.
- **Two-handed crate**: Slots `Equipment.Slot.Hand.Right`, Blocked Slots `Equipment.Slot.Hand.Left`.
- **Bottle of wine**: Category `Item.Consumable.Drink`, Usable (effect: raise Intoxication), Charges 4, Value 12.
- **Mansion key**: Category `Item.Key`, Tags `Key.Door.Mansion`, Weight 0.

### Example: police uniform
`DA_Item_PoliceJacket`: Category `Item.Outfit`, Weight 1.5, Grid 2x2, Max Quantity 1, Equippable fragment with Slots `Equipment.Slot.Coat` and Granted Tags `Disguise.Faction.Police`. The game's social rules then treat the wearer as a police member (see [Social](Social.md)).

## 3. Components

| Component | Goes on | Notes |
|---|---|---|
| FV Inventory Component | Player, containers, NPCs, merchants | **Items** = starting stacks (and the saved state). **Money** = starting money (and the saved state). **Apply Carry Limits**: turn off for loot containers, merchants' stock and other storage (weight and space are then ignored). See sections 3.1 to 3.5. `Set Bonuses` lets the game layer add allowance (e.g. from an Athletics attribute) |
| FV Equipment Component | Player, NPCs | **Equipped** = starting outfit (and the saved state); starting items are linked to carried stacks at BeginPlay. **Require Inventory** = only carried items can be worn. See section 3.6 |
| FV Item Receiver Component | Anything that can take items | The inventory on the same actor listens to it |
| FV Pickup Component | World items | **Item**, **Quantity**, **Instance Data**, **Destroy Owner On Pickup**. See section 4 |
| FV Loot Component | Containers, bodies | **Table**, **Owner Tag**. Fills the inventory on the same actor once. See section 5 |

### 3.1 Inventory stacks
- Every entry is a stack with an **Instance Id**, the item, a quantity and optional **Instance Data**. Units of an item without instance data merge into its stacks up to **Max Quantity**, then start new stacks. Each stack claims the item's footprint in cells.
- **Instance data** makes copies of an item differ (dye, wear, a stolen flag, story customization). Derive a struct from `FFVItemInstanceFragment` and mark its fields `SaveGame` so they are saved. Stacks with instance data never merge. `Add Stack` adds one with its data (whole or not at all); `Set Instance Data` replaces a stack's data; `Get Stack` reads one by id.
- The plugin ships two (in `Inventory/FVItemInstanceData.h`): **Charges Left** (`Remaining` uses on a partly used unit) and **Stolen** (`Owner`: the faction or character Id it was taken from; empty = unknown owner).
- Item-based calls (`Add Item`, `Remove Item`, `Get Quantity`, `Has Item`, `Use Item`) work across all stacks of the item. `Remove Item` takes from unworn plain stacks first. `Remove Stack` removes from one stack.
- Tag-based calls read the item definitions' **Tags**: `Find Item With Tag(Tag)` returns the first stack whose item has the tag (or a child of it), `Get Quantity With Tag(Tag)` counts the units.
- Events: **On Item Changed** (totals of an item), **On Stack Changed** (a stack was added, changed or removed; `Get Stack` fails for a removed one), **On Inventory Reset** (contents replaced by a load; rebuild the UI from `Get Items`), **On Capacity Changed**, **On Item Used**, **On Money Changed** (old and new money; also fired with equal values after a load).
- Space is a cell count: pockets plus equipped containers plus bonuses. Items have no positions; the UI lists them and shows cells used.

### 3.2 Using items
- `Use Stack(Id)` applies the item's Usable effects to the owner (Instigator and Target), then:
  - with a **Charges** fragment it spends one use. A partly used unit is its own stack carrying **Charges Left**: using one unit of a stack of three bottles splits it off. The unit goes after its last use.
  - without Charges it consumes one unit if **Consume On Use**.
- `Can Use Stack(Id)`: the item is usable, its Use Conditions pass, and a unit that would be used up can leave the inventory (a worn bag can't if the rest wouldn't fit).
- `Use Item(Item)` / `Can Use Item(Item)` pick the stack for you: a partly used unit first, then unworn plain stacks.
- `Get Charges(Id)`: uses left on the stack's next unit (its Charges Left, else the item's Charges, else 1 for a usable item, 0 otherwise).
- An item lying in the world is used by the same rules through its pickup (`Use In Place`, section 4). A world stack doesn't split: its Charges Left is for its next unit and comes along when it is picked up; picking it up puts the partly used unit in its own stack.

### 3.3 Money
Money is a number on the inventory component, not an item; coins found, looted or stolen add to it.
- `Get Money`, `Can Afford(Amount)`, `Add Money(Amount)` (ignores 0 or less), `Remove Money(Amount)` (false and no change if there isn't enough), `Transfer Money(To, Amount)` (moves Amount or all there is, returns the amount moved).
- Bind **On Money Changed** for the HUD.

### 3.4 Keys and Quest list
- `Get Storage Items` lists everything but key items; `Get Key Items` lists the Keys and Quest list. `Get Items` still returns every stack.
- Key items never count toward weight or cells and are always accepted, even when the inventory is full.
- Which categories go into the list is the **Key Item Categories** setting (section 7).

### 3.5 Moving stacks between inventories
- `Transfer Stack(To, Id, Quantity)` moves up to Quantity units of an unworn stack and returns the units moved. Plain units merge on arrival and may move in part (as far as they fit); a stack with instance data keeps it and moves whole (up to its item's Max Quantity per stack) or not at all.
- `Transfer Stack As Stolen(To, Id, Quantity, Stolen From)` does the same and gives the moved units the **Stolen** mark with that owner, so they never merge with legally owned copies.
- `Transfer All(To, Include Money, Stolen From)` moves every unworn stack, and the money if asked; a valid Stolen From marks everything moved. Returns the number of stacks moved whole.
- Worn stacks never move.

### 3.6 Equipment
- `Equip(Item)` wears a carried, unworn stack in the first of its slots that is free; if none is free it uses the first slot. `Equip Instance(Id, Slot)` wears a specific stack; Slot None picks as above, a named slot moves a worn stack there.
- Whatever conflicts comes off: the item in the chosen slot, items in the new item's Blocked Slots, and items whose Blocked Slots cover the chosen slot. `Get Blocking Item(Slot)` tells the UI why a slot is unavailable.
- Items held in a hand use the `Equipment.Slot.Hand.Left` and `.Hand.Right` slots; a two-handed item lists the other hand in its Blocked Slots.
- **Overflow rule**: capacity never shrinks below what is carried. Taking off or swapping a bag or backpack (`Can Unequip`, `Can Equip`), or dropping a worn one (`Remove Item`/`Remove Stack`), is refused while the remaining space or weight allowance can't hold the carried items. Limits that are switched off, or an inventory without carry limits, don't count. Capacity lowered by `Set Bonuses` isn't checked.
- A worn stack that leaves the inventory is taken off.
- Granted tags are applied to the ASC as loose tags; only the difference is added or removed on each change. Call `Refresh Granted Tags` if the ASC appears after BeginPlay.
- Events: **On Equipment Changed** (slot, old item, new item; one per affected slot, after tags are updated), **On Equipment Reset** (all slots replaced by a load).

### 3.7 Script helpers
Blueprint and AngelScript can't read instanced structs, so `UFVItemStackLibrary` (`FVItemStack::` in AngelScript) reads them: `Is Stolen(Stack, Out Owner)`, `Get Charges Left(Stack)`, `Get Base Price(Item)` (0 without a Value fragment), `Is Sellable(Item)` (Value fragment marked Sellable). In C++, `SpendUse(Stack)` spends one use of a stack held outside an inventory, by the rules `Use Stack` and `Use In Place` share.

## 4. Pickups

**FV Pickup Component** holds a stack: **Item**, **Quantity** and **Instance Data** (e.g. a Stolen mark or Charges Left). Quantity and Instance Data are saved with an **FV Saveable Component** on the actor.
- `Try Pickup(Picker)` offers the stack to the picker's receiver. A plain stack that doesn't fully fit leaves the rest in the world; a stack with instance data goes in as whole stacks of at most Max Quantity, as many as fit (the rest stays in the world); a partly used unit (Charges Left) goes in first as its own stack, and the full units after it go in as plain units.
- **On Picked Up** fires after the quantity is updated: `Get Quantity` is what is left.
- `Can Use In Place(User)` / `Use In Place(User)` use a usable item where it lies, by the rules of `Use Stack` (section 3.2) with the user as Instigator and Target. **On Used In Place** fires after the stack is updated.
- With **Destroy Owner On Pickup** (default) the actor is destroyed once everything is taken or used up. Turn it off (`Set Destroy Owner On Pickup(false)`) to keep the actor: the pickup is then **depleted** (`Is Depleted`, quantity 0) and can't be picked up again; the actor decides what to do, e.g. hide itself.
- `Set Stack(Stack)` / `Get Stack` set and read the whole stack, e.g. when a dropped stack is spawned into the world.

The receiver: `Receive Stack(Stack, Source)` fires **On Stack Received** (instance data included) and, for plain stacks only, **On Item Received**; `Receive Item(Item, Quantity, Source)` offers a plain stack. Handlers call `Report Taken(Quantity)`; when nobody reports, a bound listener is assumed to take everything (e.g. a quest trigger). The inventory listens to On Stack Received.

`BP_Pickup_<Name>` (Actor):
1. A static mesh tagged `Detectable`.
2. **FV Pickup Component** with the item and quantity.
3. **FV Interactable Component** with an interactable definition offering `Interaction.Action.Pickup`, whose effect activates the pickup ability.

The game layer ships a generic world item actor that takes its mesh from the item definition; see the game's Items guide (`Docs/Setup/Items.md`). Make specific Blueprints only for special meshes or behaviour.

## 5. Loot

| Asset | Class | Key fields |
|---|---|---|
| `DA_Loot_<Name>` | `FVLootTableDefinition` | **Entries**, **Min Money**, **Max Money** |

Each entry gives an **Item** or rolls a nested **Table** (the table wins if both are set), with **Min/Max Quantity** (units given, or times the nested table is rolled), **Chance** (0 to 1) and **Conditions** (Instigator = who loots, Target = the looted actor or the receiver of a Give Loot effect). Money is rolled between Min and Max Money, once per roll of the table.

Keep at most three entries per table and nest tables for more (rule of three). *Validate Data* warns past three entries and about an entry with both an Item and a Table, and fails on an entry with neither, on a table rolling itself, and on Min above Max. Nesting stops at 8 levels at runtime.

**FV Loot Component** on a container or body that also has an **FV Inventory Component** (Apply Carry Limits off):
- **Table**: rolled into the inventory once, by `Roll If Needed(Instigator)` (true if it rolled now). `Has Rolled` is saved, so a looted container stays looted.
- **Owner Tag**: the faction or character Id owning the contents; empty = free to take. Pass it as Stolen From when moving items out (`Transfer All`, `Transfer Stack As Stolen`). `Get Owner Tag` reads it.

From Blueprint/AngelScript use `UFVLootLibrary` (`FVLoot::` in AngelScript): `Roll Loot(Table, Instigator, Target)` returns the items (one plain stack per item) and money without giving them; `Give Loot(Table, Inventory, Instigator)` rolls with the inventory's owner as Target, adds the items as far as they fit and the money, and returns the units added.

### Example: police locker
`DA_Loot_PoliceLocker`: Min Money 5, Max Money 30, Entries: `DA_Item_Donut` x1-2 (Chance 0.5), `DA_Loot_PoliceGear` x1. The locker has an inventory (Apply Carry Limits off) and a loot component with that table and the police faction's Id as Owner Tag.

## 6. Conditions and effects

| Conditions | Effects |
|---|---|
| Has Item (quantity; Instigator or Target), Has Item With Tag (units of items whose definition Tags have the tag, e.g. a key or a tool), Has Money (at least Amount), Is Wearing (an item, or anything granting a tag) | Give Item (optionally equip; ignores carry limits so a reward is never lost), Take Item, Give Money, Take Money (nothing if there isn't enough; gate it with Has Money), Give Loot (rolls a loot table into the Instigator's or Target's inventory) |

Use *Is Wearing* with a granted tag for dress codes ("wear anything granting `Disguise.Faction.Police`").

## 7. Settings

*FlickerVoid > Inventory*:

| Setting | Meaning |
|---|---|
| Enforce Weight / Enforce Space | Switch each limit on or off |
| Base Weight Limit | kg carried with nothing equipped |
| Base Pocket Cells | Cells available without a bag |
| Key Item Categories | Item categories kept in the Keys and Quest list, without weight or cells. Default `Item.Key`, `Item.Quest` (in `Config/DefaultGame.ini`) |

## 8. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Inventory.Debug.HUD 1` | Carried items, weight, space and money on screen; stacks are marked `(custom)`, `(key)`, `(stolen)` and `(worn)` |
| `FVCvar.Inventory.EnforceWeight` / `EnforceSpace` | -1 use settings, 0 off, 1 on |
| `FV.Inventory.Give <ItemIdOrAsset> [Quantity]` | Give the player an item |
| `FV.Inventory.Equip <ItemIdOrAsset>` | Equip a carried item |
| `FV.Inventory.Use <ItemIdOrAsset>` | Use a carried item |
| `FV.Inventory.GiveMoney <Amount>` | Give the player money; a negative amount takes it |
| `FV.Inventory.Loot <LootTableIdOrAsset>` | Roll a loot table into the player's inventory |

## Notes

- Inventory contents, money and equipment, including instance ids and instance data, are saved when the actor also has an **FV Saveable Component** (see FVStorySystem, section 5). After a load both components rebuild capacity and granted tags and broadcast **On Inventory Reset**, **On Money Changed** and **On Equipment Reset**.
- Outfit meshes are not applied by the plugin; bind **On Equipment Changed** and **On Equipment Reset** in the game layer and set the mesh from the Equippable fragment.
- Prices, trading, giving, held items, throwing and the world item actors are game layer; see `Docs/Setup/Items.md`.
