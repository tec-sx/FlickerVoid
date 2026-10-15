# Items setup (game layer)

How items work in FlickerVoid on top of the plugins: the item types in data, the world actors (items, notes, money, loot containers), the item abilities and their animations, keys, held and thrown items, intoxication, trading, pickpocketing and giving from dialogue.

Builds on FVInventorySystem (items, inventory, money, loot; read [its guide](FVInventorySystem.md) first), FVInteractionSystem (offers, locks), FVStorySystem (knowledge), FVSocialSystem (factions, gifts) and FVAttributeSystem (attributes, checks). C++ lives in `Source/FVGameplay/.../Items` (module `FlickerVoidGameplay`), AngelScript in `Script/Items`.

## 1. Quick start

1. *Project Settings > FlickerVoid > Items*: set **World Item Class** to `AFVWorldItem` (or a Blueprint of it), and a **Default Interactable** with a Pickup offer (sections 5 and 13).
2. Make an item definition with a **World** fragment (its mesh), e.g. `DA_Item_Apple`.
3. Grant the player the item abilities (section 6) and add **UFVHeldItemComponent** to the player (section 8).
4. Place an `AFVWorldItem` in the level and set its **Item**. Its mesh and offers come from the definition.
5. Press Play, look at it and take it.

## 2. Item types in data

An item type is a category tag plus fragments; there are no item classes. Behaviour comes from the fragments and from the abilities the offers start.

| Type | Category | Fragments and fields | Behaviour |
|---|---|---|---|
| Drink, food, medicine | `Item.Consumable.Drink` / `.Food` / `.Medicine` | **Usable** (effects), optional **Charges** (sips), **Value**, **World**, **Animation** (Use Style, e.g. `Animation.Item.Use.Drink`) | Used from the inventory or straight from the world; the effects apply on the use montage's Apply notify |
| Alcohol | `Item.Consumable.Drink` | Usable with *Modify Attribute* Intoxication +N | Wears off with game time (section 11) |
| Sobering item | `Item.Consumable.*` | Usable with *Modify Attribute* Intoxication -N | |
| Key, keycard | `Item.Key` | Tags `Key.<...>` (e.g. `Key.Door.Mansion`), Weight 0 | Keys and Quest list; opens locks whose Key Tag it carries (section 10) |
| Quest item | `Item.Quest`, **Quest Item** ticked | | Keys and Quest list; never dropped, thrown, sold or stolen by pickpocketing |
| Valuable | `Item.Valuable` | **Value** (Base Price, Sellable) | Sold to merchants (section 12) |
| Outfit | `Item.Outfit` | **Equippable** | Worn with the Wear offer |
| Held item | `Item.Held` | **Equippable** with Slots `Equipment.Slot.Hand.Right` and/or `.Hand.Left` (two-handed: the other hand in Blocked Slots), **World** (Actor Class for e.g. a flashlight), **Animation** (Hold Style) | Held with the Hold offer; used with Use Held (section 8) |
| Throwable | `Item.Throwable` | **Throwable**, **World** | Thrown to make noise (section 9) |
| Tool, credential | any | Tags `Tool.Lockpick`, `Tool.Credential.Police`... | Read by *Has Item With Tag* conditions on offers |

Not items:
- **Money** is a number on the inventory component. Coins in the world are an `AFVMoneyPickup`.
- **Notes, letters, books, maps** are knowledge (Kind **Document**) shown by an `AFVReadable` (section 7). They never enter the inventory.

There is no energy system; what a consumable does is only its Usable effects.

## 3. Item fragments (game module)

Added to an item definition's **Fragments** like the plugin's own.

| Fragment | Fields |
|---|---|
| World | **Mesh** (in the world and in the hand), **Actor Class** (spawned instead of the generic world item when dropped or held, e.g. a flashlight with a light), **Interactable** (overrides the offers chosen by category), **Grip Socket** (socket on the item mesh placed in the hand; none = mesh origin), **Grip Offset** (from the hand socket), **Pickup Sound**, **Drop Sound**, **Use Sound** |
| Animation | **Use Style** (under `Animation.Item.Use`), **Hold Style** (under `Animation.Item.Hold`), **Use Montage** (overrides the use style's montage) |
| Throwable | **Loudness**, **Noise Range** (cm), **Throw Speed** (cm/s), **Break On Impact**, **Impact Sound** |

On a **character** definition, the **Item Animations** fragment maps styles to that character's animations: **Montages** (`Animation.Item.*` style to montage: use styles, pickup low/mid/high, give, throw) and **Hold Poses** (hold style to upper body pose). See section 6.2.

Script reads fragments through `FVItem::GetWorldFragment`, `GetAnimationFragment` and `GetThrowableFragment`.

## 4. World actors

All four are AngelScript actors in `Script/Items/World`. Place them directly, or make Blueprints of them for meshes and sounds only. Each has a mesh root tagged `Detectable`, an **FV Interactable Component**, an **FV Interactable Highlight Component** and an **FV Saveable Component**.

| Actor | Set | Behaviour |
|---|---|---|
| `AFVWorldItem` | **Item**, **Quantity** (instance only), **Appear Conditions** (checked with the player as instigator, again whenever facts change), **Simulate Physics** | The stack sits on its **FV Pickup Component**; the mesh and the interactable definition come from the item (section 5). Picked up, used up or emptied, a placed item hides and stays collected after a load; a dropped or thrown one is destroyed. `Get Stack`, `Consume One` |
| `AFVReadable` | **Knowledge** (the note, Kind Document), **Hide When Read**; its **Secrets** component (section 7) | `Mark Read(Reader)` learns the knowledge and broadcasts `Message.Item.Read`. `Needs Investigation`, `Is Solved` |
| `AFVMoneyPickup` | **Amount**, **Pickup Sound** | The pickup ability calls `Collect(Collector)`: adds Amount to the collector's money |
| `AFVLootContainer` | **Storage** (inventory: starting items and money; Apply Carry Limits off), **Loot** (table, **Owner Tag**), **Interaction Point** (where the hand reaches; motion warping target). Add an **FV Lock Component** on the instance or a Blueprint for a lock | `Try Open(Opener)`: unlocks with a carried key if locked, rolls the loot once, broadcasts `Message.Item.ContainerOpened`. `Take All(Taker)`: moves items and money to the taker; with an Owner Tag they get the Stolen mark and a `Message.Item.Stolen` per item |

Dropped and thrown items are spawned with `FVItem::SpawnWorldItem(Stack, Transform)`: the World fragment's Actor Class, else the settings' **World Item Class**, with the stack on its pickup component (instance data such as a Stolen mark kept).

## 5. Interactable definitions and offers

World items get their offers from the item: the World fragment's **Interactable**, else the settings' **Interactables By Category** entry of the most specific matching category, else **Default Interactable**. The other actors take the definition set on their interactable component.

Every offer's effect is **Activate Ability** with the event tag below; the ability does the work (section 6).

| Asset (suggested) | Used by | Offers: action tag, Activate Ability event |
|---|---|---|
| `DA_Interactable_Item` (Default Interactable) | Any world item | Take: `Interaction.Action.Pickup`, `Event.Item.Pickup` |
| `DA_Interactable_Consumable` (category `Item.Consumable`) | Drinks, food, medicine | Take; Use: `Interaction.Action.Use`, `Event.Item.Use` |
| `DA_Interactable_Outfit` (category `Item.Outfit`) | Clothes | Take; Wear: `Interaction.Action.Wear`, `Event.Item.Wear` |
| `DA_Interactable_Held` (category `Item.Held`) | Flashlights, crates | Take; Hold: `Interaction.Action.Hold`, `Event.Item.Hold` |
| `DA_Interactable_Readable` | `AFVReadable` | Read: `Interaction.Action.Read`, `Event.Item.Read`, condition *Script > FV Needs Investigation Condition* with **Invert**; Investigate: `Interaction.Action.Investigate`, the event of your Examine offer, condition *Script > FV Needs Investigation Condition* |
| `DA_Interactable_Money` | `AFVMoneyPickup` | Take: `Interaction.Action.Pickup`, `Event.Item.Pickup` |
| `DA_Interactable_LootContainer` | `AFVLootContainer` | Take All: `Interaction.Action.TakeAll`, `Event.Item.TakeAll` |
| Character interactables | NPCs | Pickpocket: `Interaction.Action.Pickpocket`, `Event.Item.Pickpocket` (NPCs with an inventory). Merchants: Trade: `Interaction.Action.Trade`, effect *Script > FV Trade Effect* (no ability) |

Interactable types: `Interactable.Item.Pickup` for items and money. `Interaction.Action.Search` is reserved for the take screen (section 16).

## 6. Abilities

AngelScript abilities in `Script/Items/Abilities`, all derived from `UFVInteractAbility`. Each sets its **Trigger Event Tag** in script, so no Ability Trigger is needed; the project ASC (`UFVAbilitySystemComponent`) registers it when the ability is granted. An event sent by an offer carries the interactable; an event sent with `FVItem::SendItemEvent` carries an item payload (item, instance id, world actor, other actor, quantity).

| Ability | Event | Started by | Does |
|---|---|---|---|
| `UFVPickupAbility` | `Event.Item.Pickup` | Take offer | Money: collected at once. Items: pickup montage by height (`Animation.Item.Pickup.Low/Mid/High`, else its **Pickup Montage**), warped to the item, taken on Apply |
| `UFVUseItemAbility` | `Event.Item.Use` | Use offer (world), inventory stack (payload), Use Held | Use montage (`FVItem::FindUseMontage`) with a prop in the right hand from Attach to Release; the item takes effect on Apply (`Use Stack` from the inventory; effects and one unit off the world item otherwise) |
| `UFVWearItemAbility` | `Event.Item.Wear` | Wear offer, inventory stack | Picks a world item up first, then wears it |
| `UFVHoldItemAbility` | `Event.Item.Hold` | Hold offer, inventory stack | Puts the stack in a hand slot: the right hand if it fits (the left when only that one is free), else the left; holding a held stack again puts it away |
| `UFVUseHeldItemAbility` | `Event.Item.UseHeld` | Input tag | Uses what is held, right hand first (section 8) |
| `UFVDropItemAbility` | `Event.Item.Drop` | Inventory stack (payload: instance id, quantity) | Spawns the units in front of the character with physics; never worn, held, quest or key items |
| `UFVThrowItemAbility` | `Event.Item.Throw` | Input tag, inventory stack | Throws one unit of the payload stack or the held throwable (section 9). **Aim Pitch**, **Release Distance** |
| `UFVReadAbility` | `Event.Item.Read` | Read offer | Read montage (`Animation.Item.Use.Read`), then `Mark Read` on Apply |
| `UFVTakeAllAbility` | `Event.Item.TakeAll` | Take All offer | Reach montage (`Animation.Item.Pickup.Mid`) warped to the container's Interaction Point, then `Take All` on Apply. A locked container needs a fitting key |
| `UFVPickpocketAbility` | `Event.Item.Pickpocket` | Pickpocket offer | Section 14 |
| `UFVGiveItemAbility` | `Event.Item.Give` | FV Give Item flow node | Section 15 |
| `UFVExamineAbility` (`Script/Abilities`) | your Examine event | Examine and Investigate offers | Examine view; finds secrets (section 7) |

Grant every ability once, in the player's **Ability Set**: under *Granted Interaction Abilities* with the matching `Interaction.Action` tag, or under *Granted Gameplay Abilities* with an **Input Tag** for the ones on a key (Throw, Use Held). Granting one twice makes an event start it twice. NPCs need none: the player runs the give and pickpocket abilities.

Settings that live on an ability (**Pickup Montage**, the pickpocket's **Steal Check**, the give's **Give Scene**) are set on a Blueprint child of the ability, which the ability set then grants.

Abilities that need both hands own `Status.HandsBusy`: held items are put away while they run, and Throw and Use Held can't start.

### 6.1 Montages and notifies

Item montages carry **FV Gameplay Event** notifies (the `UFVAnimNotify_GameplayEvent` class; the notify shows the tag's last part):

| Event | Moment |
|---|---|
| `Event.Item.Notify.Attach` | The item reaches the hand: its prop appears |
| `Event.Item.Notify.Apply` | The item takes effect: used, taken, thrown, read or looted |
| `Event.Item.Notify.Release` | The item leaves the hand: its prop goes |
| `Event.Item.Notify.Handover` | Give scenes: the prop passes to the other character's hand |

Without an Apply notify the action happens when the montage completes; without a montage it happens at once. A montage interrupted before Apply does nothing (nothing is used or taken).

Montages that walk or turn to the target use a **motion warping** window named `Item`; the ability sets its target (the world item, the readable, or the container's Interaction Point). The character needs a **Motion Warping** component (`APlayerCharacter` has one).

### 6.2 Animation styles

Styles are tags: `Animation.Item.Use` (children are data, e.g. `Animation.Item.Use.Drink`; `Animation.Item.Use.Read` exists for reading), `Animation.Item.Hold` (children are data), `Animation.Item.Pickup.Low/Mid/High`, `Animation.Item.Give`, `Animation.Item.Throw`.

A montage is looked up in the character definition's **Item Animations** fragment (the exact style, then its parents), then in the settings' **Default Montages** (same order). A use montage is the item's **Use Montage**, else its Use Style's montage, else `Animation.Item.Use`'s. The pickup style follows the item's height above the character's feet (**Pickup Low Height**, **Pickup High Height**).

## 7. Notes and investigation

An `AFVReadable` shows a note; its **Knowledge** (Kind **Document**) is what reading teaches.
- **Read** (`UFVReadAbility`) learns it and broadcasts `Message.Item.Read` (Reader, Knowledge, Source); the reader screen opens on it (section 16). **Hide When Read** removes the note afterwards.
- A note with secrets must be **investigated** first. Its **Secrets** component (`UFVExamineSecretsComponent`) lists up to three secrets, each a **View Angle** (rotation in the examine view) and a **Clue** (knowledge), matched within **Angle Tolerance**. The Investigate offer starts the examine ability; turning the note to a secret's angle learns its clue. When every clue is known the note is read (its knowledge learned) and the view closes. Cancelling keeps the clues found and leaves the note unsolved in the world.
- The *FV Needs Investigation Condition* (Script condition) is true while the target readable has unfound secrets; use it as in section 5 so only one of Read and Investigate shows.

Any examinable actor can have a Secrets component; only readables are solved by finding them all.

## 8. Held items

Add **UFVHeldItemComponent** (`Script/Items/Components`) to the player (and to NPCs that hold things). It watches the equipment component's `Equipment.Slot.Hand.Right` and `.Hand.Left`:
- The World fragment's **Actor Class** is spawned at the hand socket (inert: no collision, physics or offers), else a mesh prop of the World mesh. **Grip Socket** and **Grip Offset** place it.
- The hold pose of the item (**Hold Style**, looked up like montages in **Hold Poses** / **Default Hold Poses**) goes to `UFVPlayerAnimInstance.RightHandHoldPose` / `LeftHandHoldPose`.
- `Put Away` / `Take Out` hide and show what is held (abilities with `Status.HandsBusy` do it).
- `Use Held(Right Hand)`: the held actor's **UFVHeldItemBehaviourComponent**, else `Event.Item.Use` for a usable item.

**UFVHeldItemBehaviourComponent** goes on a held actor class, e.g. `BP_Flashlight`: **Toggle State** flips **On** with each use and broadcasts **On Toggled(On)** (bind the light to it); **On Used(User)** fires on every use.

AnimBP: blend each hold pose over its arm, e.g. *Layered Blend Per Bone* from `clavicle_r` (right) and `clavicle_l` (left) with a *Sequence Player* of the pose, weight 1 while the pose is set and 0 when it is none.

## 9. Throwing

Throw takes one unit off the stack (a held stack's last unit leaves the hand), spawns it at the hand socket and pushes it along the aim at **Throw Speed**. Its first hit (**UFVThrownItemComponent**, added by the ability):
- reports an AI hearing noise (`UAISense_Hearing`, tag `ItemNoise`, **Loudness**, **Noise Range**) with the thrower as instigator,
- broadcasts `Message.Item.Noise` (Instigator, Location, Loudness, Range),
- plays **Impact Sound** and destroys the item if **Break On Impact**.

## 10. Keys

Give a lock (**FV Lock Component**) a **Key Tag** under `Key.` and give the key item that tag in its definition **Tags**; tick **Consume Key** for single-use keys. `FVItemKeys::HasKeyFor(Holder, Lock)` and `FVItemKeys::TryUnlockWithKey(Holder, Lock)` (`Script/Items/FVItemKeys.as`) check the holder's inventory, unlock, and take one key unit when it is consumed.
- Doors (`Script/World/Doors/InteractionResponse_DoorToggle.as`): a toggle blocked by the lock tries the interactor's key, then opens.
- Loot containers try the key in `Try Open` and `Take All`.
- Lockpicking is unchanged.

## 11. Intoxication

Add **UFVIntoxicationComponent** (`Script/Items/Components`) to the player next to its **FV Attribute Component**:

| Property | Meaning |
|---|---|
| Intoxication | Attribute raised by alcohol, e.g. `DA_Attribute_Intoxication` (Min 0, Max 100) |
| Health | Attribute the hangover lowers |
| Sober Per Hour | Intoxication lost per game hour (default 10). Every hour counts, also when time jumps |
| Hangover Threshold | Peak intoxication from which getting sober (Intoxication 0) costs Health (default 50) |
| Hangover Health Loss | Health lost to the hangover (default 10) |

Alcohol raises Intoxication and sobering items lower it, both with *Modify Attribute* in their Usable effects. A sleep system calls `Sleep It Off` (Intoxication to 0, hangover rule included). The peak and the last counted hour are saved with the player's **FV Saveable Component**. It needs the world clock (FVFramework) running: **Seconds Per Hour** above 0, or time advanced by *Advance Time*.

## 12. Trading

A merchant is an NPC with an **FV Inventory Component** (its **Items** are the stock, its **Money** pays for what it buys; **Apply Carry Limits** off) and **UFVMerchantComponent** (`Script/Items/Trade`):

| Property | Meaning |
|---|---|
| Faction | Whose standing and disguise change prices; none = the NPC's faction (Social fragment) |
| Buy Multiplier | Base price multiplier when the player buys (default 1) |
| Sell Multiplier | Base price multiplier when the player sells (default 0.5) |
| Standing Price Factor | Price change per standing point (default 0.005: standing 100 = 50%) |
| Disguise Discount | Extra change while the player is disguised as one of the faction (default 0.1) |
| Fence | Buys goods stolen from its own faction or from itself, which other merchants refuse |

Prices start from the item's **Value** fragment: buying = Base Price x Buy Multiplier x (1 - change), at least 1; selling = Base Price x Sell Multiplier x (1 + change). The change is standing x factor plus the disguise discount, within -50% and +50%. A merchant won't buy items that aren't Sellable, quest items, or (unless a fence) items stolen from its faction or from itself.

Functions: `Get Buy Price(Item, Buyer)`, `Get Sell Price(Stack, Seller)`, `Can Buy` / `Buy(Buyer, Stock Instance Id, Quantity)`, `Buy Item(Buyer, Item, Quantity)` (stack by stack; returns the units bought), `Can Sell` / `Sell(Seller, Instance Id, Quantity)` (a Stolen mark stays on the goods), `Request Trade(Customer)` (broadcasts `Message.Item.TradeRequested`), `Get Stock`, `Get Price Faction`.

Script effects for dialogue and offers (*Script* effect, Instigator = the customer, Target = the merchant):
- **FV Buy Item Effect** (`Item`, `Quantity`): buys from the stock without the trade screen, e.g. a drink at a bar; gate the choice with *Has Money*.
- **FV Trade Effect**: requests the trade screen.

## 13. Settings

*Project Settings > FlickerVoid > Items* (`[/Script/FlickerVoidGameplay.FVItemSettings]` in `Config/DefaultGame.ini`):

| Setting | Meaning |
|---|---|
| Interactables By Category | Offers of world items by item category; the most specific matching category wins |
| Default Interactable | Offers of world items whose category has no entry |
| World Item Class | Actor spawned for dropped and thrown items: `AFVWorldItem` or a Blueprint of it |
| Drop Distance | How far in front of the character dropped items land (default 60 cm) |
| Right Hand Socket / Left Hand Socket | Character mesh sockets for held items and props (default `hand_r_item`, `hand_l_item`); add them to every character skeleton |
| Default Montages | Montages by style for characters without an entry |
| Default Hold Poses | Hold poses by style for characters without an entry |
| Pickup Low Height / Pickup High Height | Item height above the feet below which Pickup.Low and above which Pickup.High is used (default 40 and 140 cm) |

Script reads them with `FVItem::GetSettings()`.

## 14. Pickpocketing

`UFVPickpocketAbility` steals from the targeted character (it needs an inventory):
- **Steal Check** (a check definition; none = always succeeds) is rolled with **Difficulty**, the thief as instigator and the victim as target.
- Success takes money (**Min Money Percent** up to all of it) or one unit of a random stack the victim doesn't wear and the thief can carry (no quest items), each half the time when both are there. The unit gets the Stolen mark with the victim's faction Id, else their character Id. `Message.Item.Stolen` reports it (Thief, Victim, Item or Money, Owner).
- Failure is noticed: **Noticed Notoriety** with the victim's faction, and `Message.Item.Stolen` with **Noticed** set.

Being seen by others (stealth detection) comes with the stealth system.

## 15. Giving from dialogue

The **FV Give Item** Flow node (category FlickerVoid) moves an item and money between the player (the flow's instigator) and the flow's owner, usually the NPC in the conversation:

| Field | Meaning |
|---|---|
| Direction | Player Gives or Player Receives |
| Item, Quantity | What changes hands (optional when Money is set) |
| Money | Money handed over with it |
| As Gift | Player Gives only: the receiver's relationship changes by their **Gifts** fragment (FVSocialSystem), matched against the item's Tags and Category |

Pins: **In**; **Success** and **Failed**. It fails at once when the giver lacks the item or money, or the receiver's inventory can't take the item. Otherwise it sends `Event.Item.Give` to the player and waits for `Message.Item.GiveFinished`. With no ability taking the event it gives at once without animation. Items change hands as plain units: instance data (Charges Left, a Stolen mark) is dropped.

`UFVGiveItemAbility` plays the give:
- **Give Scene** (a Contextual Anim Scene asset with the roles `Giver` and `Receiver`), when both characters have a **Contextual Anim Scene Actor** component (`APlayerCharacter` has one). The item changes hands when the scene ends; **Scene Timeout** (default 20 s) ends it if the scene never reports its end. Put Attach, Handover and Release notifies in both roles' montages; the ability reads the player's.
- Else the player's give montage (`Animation.Item.Give`): the item changes hands on Apply, and its prop shows in the player's hand while the player gives or after the player receives.
- Else at once.

It always broadcasts `Message.Item.GiveFinished` (Giver, Receiver, Item, Quantity, Money, Succeeded), also when the give fails or is cancelled, so the node never waits forever.

## 16. Gameplay messages

Sent through the GameplayMessageSubsystem for UI and other systems.

| Channel | Payload | Sent when |
|---|---|---|
| `Message.Item.Read` | `FFVItemReadMessage` (Reader, Knowledge, Source) | A note is read |
| `Message.Item.GiveFinished` | `FFVItemGiveMessage` (Giver, Receiver, Item, Quantity, Money, Succeeded) | A give ends, given or not |
| `Message.Item.Stolen` | `FFVItemStolenMessage` (Thief, Victim, Item, Money, Owner, Noticed) | Pickpocketing, or taking owned contents from a container |
| `Message.Item.Noise` | `FFVItemNoiseMessage` (Instigator, Location, Loudness, Range) | A thrown item lands |
| `Message.Item.ContainerOpened` | `FFVItemContainerMessage` (Opener, Container) | `AFVLootContainer::Try Open` succeeds |
| `Message.Item.TradeRequested` | `FFVItemContainerMessage` (Opener = customer, Container = merchant) | `Request Trade` |

## 17. Script helpers

| Namespace | Functions |
|---|---|
| `FVItem::` (`UFVItemLibrary`, C++) | Fragments, `Load World Mesh`, `Load Sound`, `Find Interactable`, `Find Montage`, `Find Use Montage`, `Find Hold Pose`, `Get Pickup Style`, `Get Hand Socket`, `Can Use Item`, `Apply Use Effects`, `Can Give Item`, `Give Item`, `Spawn World Item`, `Send Item Event`, `Get Item Payload`, `Get Settings` |
| `FVItemAnim::` (`Script/Items/Abilities/FVItemAbilityHelpers.as`) | Montage and notify tasks, motion warping targets, hand props, hand slots, `Set Hands Busy` |
| `FVItemWorld::` (same file) | Event target, sounds, world item of an actor, picking a world item up, drop transform, physics |
| `FVItemKeys::` (`Script/Items/FVItemKeys.as`) | `Has Key For`, `Try Unlock With Key` |

`UFVItemMontageAbility` is the base of the montage-playing item abilities: override `On Item Attach`, `On Item Apply`, `On Item Release` and `On Item Ability Ended`.

## 18. Assets checklist

Content isn't in this repository; make these in the editor.
- **Settings**: Items (section 13), including the hand sockets and default montages and hold poses.
- **Interactable definitions** with their offers and Activate Ability event tags (section 5); category entries in Interactables By Category.
- **Ability set** entries for the player (section 6), and Blueprint children of the abilities that need data (Steal Check, Give Scene, Pickup Montage).
- **Attributes**: Intoxication and Health; the player's Intoxication component (section 11).
- **Character definitions**: Item Animations fragment (montages, hold poses), Gifts fragment, Social fragment (faction, relationship fact) for gift receivers, pickpocket victims and merchants.
- **Montages** with FV Gameplay Event notifies (Attach, Apply, Release, Handover) and a motion warping window named `Item`; optionally a Contextual Anim Scene for giving.
- **Skeletons**: sockets `hand_r_item` and `hand_l_item`.
- **AnimBP**: hold pose blend (section 8).
- **Actors**: Blueprints of the world actors if needed, a `BP_Flashlight` (or similar) with a Held Item Behaviour component, merchants with inventory and merchant components.
- **Items**: World fragments (meshes, sounds) on every item that appears in the world.

## 19. Retired scripts

These scripts were removed; Blueprints that use them no longer compile:

| Removed | Was | Replace with |
|---|---|---|
| `Script/World/Items/Item.as` | `AItem` | `AFVWorldItem` (set Item) |
| `Script/World/Items/RigidItem.as` | `ARigidItem` | `AFVWorldItem` with Simulate Physics |
| `Script/World/Items/Pickups/BeverageItem.as` | `ABeverageItem` | `AFVWorldItem` with a drink item (Usable, Use offer) |
| `Script/World/Items/InteractionResponse_Consume.as` | `UInteractableResponseComponent_Consume` | A Use offer (`Event.Item.Use`) |
| `Script/Interaction/Requirements/FVItemRequirement.as` | `UFVItemRequirement` (already commented out) | *Has Item* / *Has Item With Tag* conditions |

Check every Blueprint deriving from `AItem`, `ARigidItem` or `ABeverageItem`, or holding a `UInteractableResponseComponent_Consume`, and reparent or rebuild it. `UFVPickupAbility` moved to `Script/Items/Abilities` with its name unchanged. `UFVExamineAbility` lost **Secret View Angles**, **Secret Knowledge** and **Secret Angle Tolerance**: move such data into a Secrets component on the examined actor.

## 20. Not in this work

- **Screens and hub actions**: the take screen (and the Search offer), the trade screen, the reader screen and the hub menu's Use, Wear, Hold and Drop actions come in a follow-up after the hub menu PR (#55). Everything they need is here: inventory functions and delegates, the merchant functions, `FVItem::SendItemEvent` for the item events, and the messages in section 16.
- **Character and outfit visuals**: worn outfits don't change the character's meshes yet.
- **Stealth reactions**: noticing theft by onlookers and AI reactions to `ItemNoise` come with the stealth system.
