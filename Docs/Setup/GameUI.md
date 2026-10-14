# Game UI setup

The game's HUD, hub menu (Tab), world map screen (M), main menu, pause menu (Esc) and options. All logic is in AngelScript under `Script/UI` (and `Script/Core/Player/PlayerCharacterController.as`); the owner makes the Widget Blueprints, input actions and settings below. Widget Blueprints only add layout and looks: parent them to the listed class and name the bound widgets exactly as listed (**required** names must exist, *optional* ones may be left out).

Read [FVFramework](FVFramework.md) section 3 first (layers, layout, HUD class).

## Rule of three

At most three of anything: HUD areas, hub pages, page sections, filter tabs, menu entries, quality options, key hints, notifications on screen. Arrays meant to stay small carry `meta = (MaxEntries = 3)`; *Data Validation* warns when one holds more.

## 1. Layers

`UI.Layer.HUD`, `UI.Layer.Game`, `UI.Layer.GameMenu`, `UI.Layer.Menu` and `UI.Layer.Modal` are declared in code (FlickerVoidUI). `WBP_FVUILayout` must register all five (FVFramework section 3).

| Layer | Holds |
|---|---|
| HUD | `WBP_GameHUD`, pushed once by the HUD actor |
| Game | Interaction prompt (FVInteractionSystem, from `DA_InteractionUISettings`) |
| GameMenu | `WBP_HubMenu`, `WBP_MapScreen` |
| Menu | `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_Options` |
| Modal | Free for confirmations |

## 2. Input

Two mapping contexts: `IMC_UI` is part of gameplay and opens screens; `IMC_Menu` is added only while a screen is open and drives the screens.

| Asset | Type | Settings |
|---|---|---|
| `IA_HubMenu`, `IA_Map`, `IA_Pause` | Input Action | Digital (bool) |
| `IA_Menu_NextPage`, `IA_Menu_PreviousPage`, `IA_Menu_Back` | Input Action | Digital (bool) |
| `IMC_UI` | Input Mapping Context | Tab → IA_HubMenu, M → IA_Map, Escape → IA_Pause |
| `IMC_Menu` (in `FVFramework/Content/Input`, with its actions) | Input Mapping Context | Tab → IA_Menu_NextPage, E → IA_Menu_NextPage, Q → IA_Menu_PreviousPage, Escape → IA_Menu_Back, M → IA_Map (add gamepad keys as wanted) |

1. In `DA_InputConfig` add to **Native Input Actions**: `IA_HubMenu` with `InputTag.UI.HubMenu`, `IA_Map` with `InputTag.UI.Map`, `IA_Pause` with `InputTag.UI.Pause`. The `IMC_Menu` actions don't go here.
2. In the player controller Blueprint add `IMC_UI` to **Default Input Mappings**.
3. Set *Project Settings > FlickerVoid > UI > Menu Input Context* to `IMC_Menu` (priority 100 by default, above the gameplay contexts).
4. *Project Settings > Common Input Settings > Enable Enhanced Input Support* must be on (this PR sets it in `DefaultGame.ini`).
5. Fill **Action Description** on each action: the Options controls list and the key hints show it.

How it plays:
- An open screen (CommonUI *Menu* mode) blocks gameplay input, so the player controller no longer hears Tab, M or Escape. Instead, while any screen is open `IMC_Menu` is active and each screen receives its actions through CommonUI action bindings (`Bind Input Action` on `FVActivatableWidget`). These work whatever has keyboard focus, so clicking around doesn't break the keys.
- In the hub, Tab and E go to the next page, Q to the previous one, and Escape closes it. On the map, M or Escape closes it. In the pause and options menus, Escape goes back.
- When the last screen closes, `IMC_Menu` is removed and the game input mode is restored, even when no Game-mode HUD is on screen.

## 3. Player controller and HUD actors

| Blueprint | Parent | Settings |
|---|---|---|
| `BP_PlayerController` (existing) | `PlayerCharacterController` | **Hub Menu Class** `WBP_HubMenu`, **Map Screen Class** `WBP_MapScreen`, **Pause Menu Class** `WBP_PauseMenu`; `IMC_UI` in Default Input Mappings |
| `BP_GameHUD` | `FVGameHUD` | **HUD Widget Class** `WBP_GameHUD`, **Options Class** `WBP_Options` (applies saved volume and subtitles at start). Set as the game mode's HUD class |
| `BP_MainMenuHUD` | `FVMainMenuHUD` | **Main Menu Class** `WBP_MainMenu`, **Options Class** `WBP_Options`. Set as the HUD class of the main menu level's game mode |

`OpenHubMenu(Page)` on the controller opens the hub on a given page class.

## 4. Shared widgets

| Widget | Parent | Bound widgets | Class defaults |
|---|---|---|---|
| `WBP_ListEntry` | `FVListEntryWidget` | **EntryButton** (Button), **TitleText**; *SubtitleText*, *ValueText*, *IconImage*, *ProgressBar* | Style the highlighted/dimmed states in **On Entry Changed**, selection in **On Selected Changed** |
| `WBP_TabButton` | `FVTabButtonWidget` | **TabButton** (Button), **LabelText** | Style selection in **On Selected Changed** |
| `WBP_TabBar` | `FVTabBarWidget` | **TabPanel** (Horizontal Box) | **Tab Button Class** `WBP_TabButton`. Hides itself with fewer than two tabs |

Make extra list rows (e.g. `WBP_QuestRow`, `WBP_KeyHint`, `WBP_Notification`) as more Blueprints of `FVListEntryWidget` when a list needs a different look.

## 5. HUD: `WBP_GameHUD`

Parent `FVGameHUDWidget` (input mode Game). Three areas, laid out as you like; the areas collapse while a GameMenu screen is open (**Hide While Game Menu Open**).

| Area (optional names) | Children |
|---|---|
| *InformationArea* | `WBP_QuestTracker`, `WBP_Notifications`, `WBP_PlayerStatus` |
| *ActionArea* | `WBP_ScannerReadout`, `WBP_KeyHints` (the interaction prompt draws on UI.Layer.Game above it) |
| *NavigationArea* | `WBP_Minimap` with its ring compass (FVNavigationSystem guide) |

| Widget | Parent | Bound widgets | Class defaults |
|---|---|---|---|
| `WBP_QuestTracker` | `FVQuestTrackerWidget` | **QuestList** (Vertical Box) | **Entry Class**, **Max Quests** (1-3). Shows active quests, last changed first, with their current objective |
| `WBP_Notifications` | `FVNotificationFeedWidget` | **NotificationList** (Vertical Box) | **Entry Class**, **Max Visible** (1-3), **Duration**, **Knowledge Fact Root** (the root of knowledge Ids, e.g. `Knowledge`), labels (Picked up, Removed, Quest started/updated/completed/failed, Title earned/lost, New note) |
| `WBP_PlayerStatus` | `FVPlayerStatusWidget` | *DisguisePanel*, *DisguiseText*, *DisguiseIcon*, *NotorietyPanel*, *NotorietyText*, *NotorietyBar*, *TitleText* | **Notoriety Scale** |
| `WBP_ScannerReadout` | `FVScannerReadoutWidget` | *TargetNameText*, *ScanProgressBar*, *EntriesText* | Shown only in scan mode (scanner component on the pawn) |
| `WBP_KeyHints` | `FVContextHintsWidget` | **HintList** (Horizontal Box) | **Hints** (up to 3: action + label, e.g. IA_HubMenu "Hub", IA_Map "Map", IA_Pause "Pause"), **Hint Entry Class** (key shows in ValueText) |

## 6. Hub menu

### `WBP_HubMenu`
Parent `FVHubMenuWidget`. Bound: **PageTabs** (`WBP_TabBar`), **PageSwitcher** (Common Animated Switcher, empty; set its *Transition Type*, *Transition Curve Type* and *Transition Duration* for the page change animation), *PageTitleText*.
Class defaults: **Pages** = `WBP_Page_Inventory`, `WBP_Page_Character`, `WBP_Page_Journal`; **Next Page Action** `IA_Menu_NextPage`; **Previous Page Action** `IA_Menu_PreviousPage`; **Close Actions** `IA_Menu_Back`. A close button can call **Close**.

### Pages
Every page: parent `FVHubMenuPageWidget` (Inventory: `FVInventoryPageWidget`). Class defaults: **Page Title**. A page is either a free layout made in its Widget Blueprint (Inventory), or built from sections: bind **SectionPanel** and *SectionTabs* (`WBP_TabBar`) and fill **Sections** (up to 3). A **Common Animated Switcher** as SectionPanel shows the sections as animated tabs (set its *Transition Type*, *Transition Curve Type* and *Transition Duration*); any other panel (Horizontal Box) shows them side by side. A plain Widget Switcher counts as "any other panel", so use the Common Animated Switcher for tabs.

Page events for the graph: **On Page Shown** (fill a free layout here), **On Page Hidden**, and **Get Page Focus Target** (override it in a free layout to return the widget that keyboard and gamepad focus should land on).

| Page | Parent | Sections | SectionPanel |
|---|---|---|---|
| `WBP_Page_Inventory` | `FVInventoryPageWidget` | none: free layout, see below | none |
| `WBP_Page_Character` | `FVHubMenuPageWidget` | `WBP_Character_Identity`, `WBP_Character_Social`, `WBP_Character_Abilities` | Common Animated Switcher + SectionTabs |
| `WBP_Page_Journal` | `FVHubMenuPageWidget` | `WBP_Journal_Quests`, `WBP_Journal_Notes`, `WBP_Journal_Memories` | Common Animated Switcher + SectionTabs |

Every section has **Section Title** (its tab label).

### List sections
Social, Abilities, Quests, Notes and Memories share `FVHubListSectionWidget`'s bindings: **EntryList** (Vertical Box in a Scroll Box), *FilterTabs* (`WBP_TabBar`), *EmptyListText*, and a detail panel: *DetailPanel*, *DetailTitleText*, *DetailSubtitleText*, *DetailValueText*, *DetailDescriptionText*, *DetailIcon*, *DetailProgressBar*. Class defaults: **Entry Class** `WBP_ListEntry`, **Empty Text**.

### Inventory page

The inventory page is laid out by hand: place the widgets below wherever the design wants them and wire them in the page's graph. Code only reads the inventory and does the actions; nothing is filled in automatically except the outfit slots.

| Widget | Parent | Bound widgets | Notes |
|---|---|---|---|
| `WBP_ItemEntry` | `FVItemEntryWidget` (FVInventorySystem) | none required | Implement **On Item Changed** (Item, bEmpty: show the icon, quantity, anything), **On Hover Changed** and **On Selected Changed** for the looks. Hovered means under the mouse or holding keyboard/gamepad focus; the mouse moves focus with it (**Focus On Hover**), so one entry is hovered at a time |
| `WBP_ItemGrid` | `FVItemGridWidget` (FVInventorySystem) | **ItemPanel**: a Uniform Grid Panel (cells placed by **Columns**) or a Wrap Box; put it in a Scroll Box for long lists | Set **Entry Widget Class** `WBP_ItemEntry`, **Cell Count** and **Columns** on each placed grid: many cells for the inventory, a few for loot. The cells show in the designer |
| `WBP_OutfitSlot` | `FVEquipmentSlotWidget` (FVInventorySystem) | none required | An item entry for one outfit slot. Set **Equipment Slot** (and *Slot Label*) on each placed slot. It shows the player's worn item there and refreshes itself; **On Slot Changed** gives the item covering the slot when it is empty (a dress over Lower Body) |

`WBP_Page_Inventory` (parent `FVInventoryPageWidget`) holds the layout, for example a `WBP_ItemGrid` named ItemGrid in a Scroll Box, `WBP_OutfitSlot`s around a figure, an information panel with the texts, icon and Use/Wear/Drop buttons, and weight and space bars. Wire it in the graph:

1. **On Page Shown** and **On Inventory Changed**: `ItemGrid > Set Items (Get Inventory Items)` (or `Get Filtered Items` for filter tabs), then refresh the weight and space (`Get Total Weight`, `Get Weight Limit`, `Get Used Cells`, `Get Cell Capacity`) and the information panel.
2. ItemGrid **On Hovered Item Changed** (Entry, Item): `Get Item Info (Item)` and put its Name, Description, Category, Icon, weights, Size and Worn Slot into the panel; show or enable the buttons from bUsable/bCanUse, bWearable/bCanWear/bCanTakeOff and bCanDrop. Entry is null when nothing is hovered: show the selected item (`ItemGrid > Get Selected Item`) or clear the panel.
3. ItemGrid **On Item Clicked** (Entry, Item): the item is now selected; open a Use/Drop prompt next to the Entry, or act straight away. The actions take the stack's Instance Id: `Use Item`, `Wear` (Slot Tag None = first free slot), `Take Off (Slot Tag)`, `Toggle Worn`, `Drop Item (Id, Quantity)`; each returns false when it can't be done, so the graph can show a message. The page fires On Inventory Changed after each, so step 1 refreshes everything.
4. Outfit slots: bind each slot's **On Clicked** (and **On Hovered** for the panel), for example from a Make Array of the slots and a For Each Loop with Bind Event. In the handler cast Entry to `FVEquipmentSlotWidget`: wear the grid's selected item in its **Equipment Slot** when `Can Wear` says it fits, otherwise `Take Off` that slot.
5. Override **Get Page Focus Target** to return `ItemGrid > Get Focus Target`, so gamepad focus starts on the selected cell, or the first one.

Get Item Info works for any stack, also ones the player doesn't carry (loot, a trader's stock); the action flags are then false. Drop spawns the item in front of the player (set *FlickerVoid > Inventory > Default Pickup Class*, see the FVInventorySystem guide). The same grid and entry widgets make loot and trade screens: give the grid the container's or trader's stacks instead.

### Character sections

| Widget | Parent | Extra bound widgets | Class defaults |
|---|---|---|---|
| `WBP_Character_Identity` | `FVCharacterIdentitySection` | *NameText*, *PortraitImage*, *LookText*, *DisguiseText*, *FameText*, *FameBar*, *NotorietyText*, *NotorietyBar*, *TitlesText* | Not disguised / no titles / nothing worn texts, **Fame Scale**, **Notoriety Scale** |
| `WBP_Character_Social` | `FVCharacterSocialSection` | (list bindings, FilterTabs required for the two tabs) | **Factions Label**, **Relationships Label**, **Disguised As Label**, **Attitude Labels** (Hostile, Unfriendly, Neutral, Friendly, Allied) |
| `WBP_Character_Abilities` | `FVCharacterAbilitiesSection` | (list bindings + FilterTabs) | **Tabs** = Skills / Perks / Kinks, each with an attribute category tag (e.g. `Attribute.Skill`, `Attribute.Perk`, `Attribute.Kink`; create the tags) |

Social lists the factions in *FlickerVoid > Social > Factions* and the characters in the new **Relationships** list there (shown once their relationship fact is set). Abilities reads the player's FV Attribute Component until the skill tree replaces it: each attribute under the tab's category is a row, value and bar from its min/max, dimmed at its minimum.

### Journal sections

| Widget | Parent | Extra bound widgets | Class defaults |
|---|---|---|---|
| `WBP_Journal_Quests` | `FVJournalQuestsSection` | *ObjectiveList* (Vertical Box) + list bindings + FilterTabs | Active / Completed / Failed labels, **Optional Label**, **Objective Entry Class** (a row; completed objectives are highlighted) |
| `WBP_Journal_Notes` | `FVJournalKnowledgeSection` | list bindings + FilterTabs | **Filters**: Clues [Clue], Topics [Topic] |
| `WBP_Journal_Memories` | `FVJournalKnowledgeSection` | list bindings | **Filters**: Thoughts [Thought] |

Quests come from the quest subsystem (tracked quests in *FlickerVoid > Quests*), knowledge from the facts through `Get Known Knowledge`.

## 7. Map screen: `WBP_MapScreen`

Parent `FVMapScreenWidget`. Bound: **WorldMap** (a `WBP_WorldMap`, see the FVNavigationSystem guide), *MapNameText*. Class defaults: **Close Actions** `IA_Menu_Back`, `IA_Map`.

## 8. Menus

| Widget | Parent | Bound widgets | Class defaults |
|---|---|---|---|
| `WBP_MainMenu` | `FVMainMenuWidget` | **MenuSwitcher** (index 0: root panel with **PlayButton**, **OptionsButton**, **QuitButton**; index 1: play panel with **ContinueButton**, **NewGameButton**, **LoadButton**, *BackButton*) | **New Game Level**, **Options Class** `WBP_Options` |
| `WBP_PauseMenu` | `FVPauseMenuWidget` | **ResumeButton**, **OptionsButton**, **QuitButton** | **Main Menu Level**, **Options Class** `WBP_Options`, **Close Actions** `IA_Menu_Back` |
| `WBP_Options` | `FVOptionsMenuWidget` | **SectionTabs** (`WBP_TabBar`), **SectionSwitcher** (index 0 Gameplay, 1 Audio and Video, 2 Controls), *SubtitlesCheck* (Check Box), *MasterVolumeSlider*, *WindowModeCombo*, *QualityCombo* (Combo Box String), *ControlsList* (Vertical Box) | Tab labels, **Master Sound Mix** + **Master Sound Class** (volume is disabled without them), **Window Mode Labels** (Fullscreen, Windowed Fullscreen, Windowed), **Quality Options** (up to 3, e.g. Low 0, Medium 2, High 3), **Control Contexts** (e.g. the gameplay IMC, `IMC_UI` and `IMC_Menu`), **Control Entry Class**, **Close Actions** `IA_Menu_Back` |

- New Game opens **New Game Level**; Quit quits; Quit to menu opens **Main Menu Level**.
- **Continue and Load are disabled**: `UFVSaveSubsystem::LoadFromSlot` restores facts and actors in the current map but doesn't travel to the saved map yet. Enable them once loading can open the save's map first.
- The pause menu pauses the game while open; Options opens on top without resuming.
- Options are saved to the `FV_Options` slot (volume, subtitles) and the game user settings (window mode, quality). Controls are listed read-only; rebinding comes later.

## 9. Checklist

- [ ] Layout registers `UI.Layer.HUD`, `Game`, `GameMenu`, `Menu`, `Modal`
- [ ] `IA_HubMenu`, `IA_Map`, `IA_Pause`, `IA_Menu_NextPage`, `IA_Menu_PreviousPage`, `IA_Menu_Back`, `IMC_UI`, `IMC_Menu`
- [ ] `DA_InputConfig` native actions with `InputTag.UI.*`; `IMC_UI` in the controller's Default Input Mappings; *FlickerVoid > UI > Menu Input Context* = `IMC_Menu`
- [ ] `BP_PlayerController` screen classes; `BP_GameHUD` and `BP_MainMenuHUD` set on their game modes
- [ ] `WBP_ListEntry`, `WBP_TabButton`, `WBP_TabBar`
- [ ] `WBP_GameHUD`, `WBP_QuestTracker`, `WBP_Notifications`, `WBP_PlayerStatus`, `WBP_ScannerReadout`, `WBP_KeyHints`
- [ ] `WBP_HubMenu`, three pages, six sections; `WBP_ItemEntry`, `WBP_ItemGrid`, `WBP_OutfitSlot` placed in `WBP_Page_Inventory`
- [ ] `WBP_MapScreen`
- [ ] `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_Options`, a main menu level and game mode
- [ ] *Inventory > Default Pickup Class* (Blueprint of the AngelScript `AItemPickup` with its interactable definition set), *Social > Relationships*, `Attribute.Skill/Perk/Kink` tags
- [ ] The untracked `Script/UI/Menus/HubMenu` stubs (`UFVHubMenuLayoutWidget`, `UFVHubPageWidget`) are superseded by `Script/UI/Hub`; delete them when no Blueprint uses them
