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
| `IA_NextPage`, `IA_PreviousPage`, `IA_Back` | Input Action | Digital (bool) |
| `IMC_UI` | Input Mapping Context | Tab → IA_HubMenu, M → IA_Map, Escape → IA_Pause |
| `IMC_Menu` | Input Mapping Context | Tab → IA_NextPage, E → IA_NextPage, Q → IA_PreviousPage, Escape → IA_Back, M → IA_Map (add gamepad keys as wanted) |

1. In `DA_InputConfig` add to **Native Input Actions**: `IA_HubMenu` with `InputTag.UI.HubMenu`, `IA_Map` with `InputTag.UI.Map`, `IA_Pause` with `InputTag.UI.Pause`. The `IMC_Menu` actions don't go here.
2. In the player controller Blueprint add `IMC_UI` to **Default Input Mappings**.
3. Set *Project Settings > FlickerVoid > UI > Menu Input Context* to `IMC_Menu` (priority 100 by default, above the gameplay contexts).
4. *Project Settings > Common Input Settings > Enable Enhanced Input Support* must be on (this PR sets it in `DefaultGame.ini`).
5. Fill **Action Description** on each action: the Options controls list, the key hints and the action bar show it.
6. Key glyphs come from CommonUI. *Project Settings > Common Input Settings > Platform Input > Windows > Controller Data* lists `DefaultGameInputDataBinds`; fill its **Input Brush Key Map** with a brush per key the game uses (Tab, M, Escape, E, Q...). Add a second controller data Blueprint (parent `CommonInputBaseControllerData`, **Input Type** Gamepad) with the gamepad buttons and add it to the same list. A key with no brush shows its name in the hint's *KeyText* instead.

How it plays:
- An open screen (CommonUI *Menu* mode) blocks gameplay input, so the player controller no longer hears Tab, M or Escape. Instead, while any screen is open `IMC_Menu` is active and each screen receives its actions through CommonUI action bindings (`Bind Input Action` on `FVActivatableWidget`). These work whatever has keyboard focus, so clicking around doesn't break the keys.
- In the hub, Tab and E go to the next page, Q to the previous one, and Escape closes it. On the map, M or Escape closes it. In the pause and options menus, Escape goes back.
- When the last screen closes, `IMC_Menu` is removed and the game input mode is restored, even when no Game-mode HUD is on screen.
- A screen's actions (Close Actions, and Next/Previous Page in the hub when it has tabs) are listed in a **Common Bound Action Bar** if one is on screen: place one in `WBP_FVUILayout` or in a screen, with **Action Button Class** `WBP_BoundActionButton`. It shows the actions of the screen that has input, with their glyphs and Action Description.

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
| `WBP_TabButton` | `FVTabButtonWidget` (a Common Button) | **LabelText** | **Style**: a Common Button Style Blueprint (e.g. `BP_TabButtonStyle`) with the normal and **selected** brushes and text styles. The tab bar handles selection; add effects in **On Selected** / **On Deselected** if wanted |
| `WBP_TabBar` | `FVTabBarWidget` (a Common Tab List) | **TabPanel** (Horizontal Box) | **Tab Button Class** `WBP_TabButton`. Leave the tab list's own next/previous tab input empty: the hub switches pages with its Next/Previous Page actions. Hides itself with fewer than two tabs |
| `WBP_TabView` | `FVTabViewWidget` | **TabBar** (`WBP_TabBar`), **TabContent** (Named Slot) | None. Make one per look if tabs differ (e.g. big page tabs, small section tabs) |
| `WBP_BoundActionButton` | `CommonBoundActionButton` (engine) | **InputActionWidget** (Common Action Widget), **Text_ActionName** (Common Text) | Style as a Common Button; used by the Common Bound Action Bar |

**Tab view.** Tabs are optional and added where a layout wants them: place a `WBP_TabView`, drop a **Common Animated Switcher** into its **TabContent** slot (set its *Transition Type*, *Transition Curve Type* and *Transition Duration*; duration 0 switches at once), and lay the tabs out as the switcher's children, at most three. The switcher's active index in the designer is the tab shown first. Tab labels come from **Tab Labels** on the placed tab view (Details panel), else from the child's *Page Title* or *Section Title*. Widgets inside the tabs belong to the layout that holds the tab view, so its bindings (BindWidget names) still find them. A plain Widget Switcher is not enough: the tab bar drives the switcher through CommonUI, which needs the Common Animated Switcher.

**Moving to the CommonUI tabs.** `WBP_TabButton`: delete the **TabButton** Button (the widget itself is the button now), keep **LabelText**, reparent if the editor asks, and move the selected look from *On Selected Changed* into its Common Button Style. In every `WBP_TabView` (hub, Character and Journal pages, Options) replace a plain Widget Switcher in **TabContent** with a Common Animated Switcher holding the same children. `WBP_KeyHints`: set **Hint Entry Class** to the new `WBP_InputHint`.

Make extra list rows (e.g. `WBP_QuestRow`, `WBP_Notification`) as more Blueprints of `FVListEntryWidget` when a list needs a different look.

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
| `WBP_KeyHints` | `FVContextHintsWidget` | **HintList** (Horizontal Box) | **Hints** (up to 3: action + label, e.g. IA_HubMenu "Hub", IA_Map "Map", IA_Pause "Pause"; an empty label uses the action's name), **Hint Entry Class** `WBP_InputHint`. Hints whose action has no key mapped are skipped |
| `WBP_InputHint` | `FVInputHintWidget` | **KeyIcon** (Common Action Widget), **LabelText**; *KeyText* | The glyph follows the device in use (section 2, step 6); *KeyText* shows the key's name while no glyph exists |

## 6. Hub menu

### `WBP_HubMenu`
Parent `FVHubMenuWidget`. Lay the pages out by hand: the hub uses every page widget placed in its layout. Bound: *PageTabView* (`WBP_TabView` with the three pages as the children of its Common Animated Switcher), *PageTitleText*. With PageTabView one page shows at a time and Tab/E, Q switch pages; without it every placed page shows. Pages, sections and tabs over three print a warning when created.
Class defaults: **Next Page Action** `IA_NextPage`; **Previous Page Action** `IA_PreviousPage`; **Close Actions** `IA_Back`. A close button can call **Close**.

### Pages
Every page: parent `FVHubPageWidget` (Inventory: `FVInventoryPageWidget`), class default **Page Title**. Nothing is bound: lay the page out by hand and place its section widgets where you want them, side by side or in a `WBP_TabView`. The page finds the sections in its layout (not inside other widgets' own layouts), refreshes them when the page or their tab is shown, and sends focus to the first section on screen with a focus target; override **Get Page Focus Target** to choose it yourself.

| Page | Parent | Sections | Layout |
|---|---|---|---|
| `WBP_Page_Inventory` | `FVInventoryPageWidget` | `WBP_Inventory_Storage`, `WBP_Inventory_Outfit`, `WBP_Inventory_Information` | Placed side by side, no tabs |
| `WBP_Page_Character` | `FVHubPageWidget` | `WBP_Character_Identity`, `WBP_Character_Social`, `WBP_Character_Abilities` | In a `WBP_TabView` |
| `WBP_Page_Journal` | `FVHubPageWidget` | `WBP_Journal_Quests`, `WBP_Journal_Notes`, `WBP_Journal_Memories` | In a `WBP_TabView` |

Every section has **Section Title** (its tab label when it sits directly in a tab view).

### List sections
Storage, Social, Abilities, Quests, Notes and Memories share `FVHubListSectionWidget`'s bindings: **EntryList** (Vertical Box in a Scroll Box), *FilterTabs* (`WBP_TabBar`), *EmptyListText*, and a detail panel: *DetailPanel*, *DetailTitleText*, *DetailSubtitleText*, *DetailValueText*, *DetailDescriptionText*, *DetailIcon*, *DetailProgressBar*. Class defaults: **Entry Class** `WBP_ListEntry`, **Empty Text**.

### Inventory sections

| Widget | Parent | Extra bound widgets | Class defaults |
|---|---|---|---|
| `WBP_Inventory_Storage` | `FVInventoryStorageSection` | (list bindings; detail optional, Information shows the selection) | **Filters** (up to 3: label, item category, wearable only; e.g. All / Clothes `wearable` / Items), **Worn Label** |
| `WBP_Inventory_Outfit` | `FVInventoryOutfitSection` | **SlotPanel** (Vertical Box or Wrap Box), *GroupTabs* (`WBP_TabBar`) | **Groups** (up to 3, e.g. Clothes: Head, Eyewear, Upper Body, Lower Body, Coat, Gloves, Shoes; Underwear: Underwear Top/Bottom, Socks; Accessories: Accessory slots, Bag, Backpack; each slot a tag and a label), **Slot Widget Class** `WBP_OutfitSlot` |
| `WBP_OutfitSlot` | `FVOutfitSlotWidget` | **SlotButton**, **SlotLabelText**, *ItemNameText*, *ItemIcon* | Style worn / blocked (covered by another item) / selected / can take the selected item in **On Slot State Changed** |
| `WBP_Inventory_Information` | `FVInventoryInfoSection` | *ItemPanel*, *NoSelectionPanel*, *ItemIcon*, *ItemNameText*, *CategoryText*, *DescriptionText*, *QuantityText*, *ItemWeightText*, *SizeText*, *StatusText*, *FeedbackText*, *UseButton*, *WearButton* + *WearButtonText*, *DropButton*, *DropQuantity* (Spin Box), *TotalWeightText*, *WeightBar*, *SpaceText*, *SpaceBar* | Labels: Wear, Take off, Worn in, Quest item; messages: cannot use / wear / drop |

How it plays: select a stack in Storage; Information shows it with Use, Wear/Take off and Drop. Clicking an outfit slot wears the selected item there if it fits (slots that can take it are flagged), otherwise selects the slot and shows what is worn in it. Drop spawns the item in front of the player (set *FlickerVoid > Inventory > Default Pickup Class*, see the FVInventorySystem guide); the spin box picks how many of a stack.

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

Parent `FVMapScreenWidget`. Bound: **WorldMap** (a `WBP_WorldMap`, see the FVNavigationSystem guide), *MapNameText*. Class defaults: **Close Actions** `IA_Back`, `IA_Map`.

## 8. Menus

| Widget | Parent | Bound widgets | Class defaults |
|---|---|---|---|
| `WBP_MainMenu` | `FVMainMenuWidget` | **MenuSwitcher** (index 0: root panel with **PlayButton**, **OptionsButton**, **QuitButton**; index 1: play panel with **ContinueButton**, **NewGameButton**, **LoadButton**, *BackButton*) | **New Game Level**, **Options Class** `WBP_Options` |
| `WBP_PauseMenu` | `FVPauseMenuWidget` | **ResumeButton**, **OptionsButton**, **QuitButton** | **Main Menu Level**, **Options Class** `WBP_Options`, **Close Actions** `IA_Back` |
| `WBP_Options` | `FVOptionsMenuWidget` | A `WBP_TabView` holding the Gameplay, Audio and Video, and Controls panels (its **Tab Labels** name them), *SubtitlesCheck* (Check Box), *MasterVolumeSlider*, *WindowModeCombo*, *QualityCombo* (Combo Box String), *ControlsList* (Vertical Box) | **Master Sound Mix** + **Master Sound Class** (volume is disabled without them), **Window Mode Labels** (Fullscreen, Windowed Fullscreen, Windowed), **Quality Options** (up to 3, e.g. Low 0, Medium 2, High 3), **Control Contexts** (e.g. the gameplay IMC, `IMC_UI` and `IMC_Menu`), **Control Entry Class**, **Close Actions** `IA_Back` |

- New Game opens **New Game Level**; Quit quits; Quit to menu opens **Main Menu Level**.
- **Continue and Load are disabled**: `UFVSaveSubsystem::LoadFromSlot` restores facts and actors in the current map but doesn't travel to the saved map yet. Enable them once loading can open the save's map first.
- The pause menu pauses the game while open; Options opens on top without resuming.
- Options are saved to the `FV_Options` slot (volume, subtitles) and the game user settings (window mode, quality). Controls are listed read-only; rebinding comes later.

## 9. Checklist

- [ ] Layout registers `UI.Layer.HUD`, `Game`, `GameMenu`, `Menu`, `Modal`
- [ ] `IA_HubMenu`, `IA_Map`, `IA_Pause`, `IA_NextPage`, `IA_PreviousPage`, `IA_Back`, `IMC_UI`, `IMC_Menu`
- [ ] `DA_InputConfig` native actions with `InputTag.UI.*`; `IMC_UI` in the controller's Default Input Mappings; *FlickerVoid > UI > Menu Input Context* = `IMC_Menu`
- [ ] Key brushes in the keyboard controller data (`DefaultGameInputDataBinds`) and a gamepad controller data
- [ ] `BP_PlayerController` screen classes; `BP_GameHUD` and `BP_MainMenuHUD` set on their game modes
- [ ] `WBP_ListEntry`, `WBP_TabButton` (Common Button Style), `WBP_TabBar`, `WBP_TabView` with a Common Animated Switcher, `WBP_BoundActionButton` and a Common Bound Action Bar
- [ ] `WBP_GameHUD`, `WBP_QuestTracker`, `WBP_Notifications`, `WBP_PlayerStatus`, `WBP_ScannerReadout`, `WBP_KeyHints`, `WBP_InputHint`
- [ ] `WBP_HubMenu`, three pages, nine sections, `WBP_OutfitSlot`
- [ ] `WBP_MapScreen`
- [ ] `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_Options`, a main menu level and game mode
- [ ] *Inventory > Default Pickup Class* (Blueprint of the AngelScript `AItemPickup` with its interactable definition set), *Social > Relationships*, `Attribute.Skill/Perk/Kink` tags
- [ ] The untracked `Script/UI/Menus/HubMenu` stubs (`UFVHubMenuLayoutWidget`, `UFVHubPageWidget`) are superseded by `Script/UI/Hub`; delete them when no Blueprint uses them
