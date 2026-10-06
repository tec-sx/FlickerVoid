# FVStorySystem setup

Quests, knowledge (clues, topics, thoughts) and saving. Quest state and knowledge live in facts, so they save with everything else and any condition can test them.

Modules: `FVStorySystem`, `FVStorySystemDebug`, `FVStorySystemEditor` (Save Inspector). Requires Flow and FVFramework.

## 1. Quick start

1. Create fact tags for the quest and its objectives (`Fact.Quest.FindTheKey`, `Fact.Quest.FindTheKey.TalkToGuard`).
2. Make a quest definition (section 2) and add it to *FlickerVoid > Quests > Quests*.
3. Start it from data (**Set Quest State** effect or Flow node, or **Auto Start When**) or call `Start Quest`.
4. Turn on `FVCvar.Story.Debug.HUD 1` and play.

## 2. Quests

| Asset | Class | Key fields |
|---|---|---|
| `DA_Quest_<Name>` | `FVQuestDefinition` | **Id** = fact holding the state (0 not started, 1 active, 2 completed, 3 failed). **Auto Start When**, **Objectives**, **Fail When**, **On Started / On Completed / On Failed** effects, optional **Flow** |

Each **objective** has a **Fact** (set to 1 when done), a **Description**, **Complete When** conditions (checked whenever a fact changes while the quest is active), **Visible When** and **Optional**. A quest completes on its own once every required (non-optional) objective is done; a quest with only optional objectives must be completed explicitly.

Two styles, mix freely:
- **Condition-driven** (non-linear): objectives complete from conditions such as *Has Item*, *Knows*, *Fact*; the quest needs no graph.
- **Graph-driven** (linear): set **Flow** to a Flow asset that runs while the quest is active. Use **Complete Objective** and **Set Quest State** nodes in it. The graph restarts from its beginning after a load, so keep it idempotent (branch on facts).

The main story can be one root Flow (world settings) using *Sub Graph* nodes per chapter and *Set Quest State*.

### Flow nodes
| Node | Use |
|---|---|
| Set Quest State | Move a quest to Active, Completed or Failed |
| Complete Objective | Set an objective fact to 1 |
| Wait For Quest State | Hold until a quest reaches a state |
| Quest State Predicate (add-on) | Branch on a quest's state |

### Conditions and effects
**Quest State** condition and **Set Quest State** effect: start a quest from a dialogue choice, unlock an offer while a quest is active, and so on.

### Quest UI (journal)
Get `UFVQuestSubsystem`, bind **On Quests Changed** (read `Get Last Changed Quest`), list `Get Quests(Active)` and show objectives where `Is Objective Visible`, ticking those where `Is Objective Complete`.

## 3. Knowledge

| Asset | Class | Key fields |
|---|---|---|
| `DA_Knowledge_<Name>` | `FVKnowledgeDefinition` | **Id** = fact written when learned (e.g. `Knowledge.Clue.BloodOnCoat`), **Kind** (Clue, Topic, Thought), **Prerequisites**, **On Learned** effects |

- Learn through the **Learn Knowledge** effect (dialogue choices, scans, examine abilities, Flow) or `UFVKnowledgeStatics::Learn` (`FVKnowledge::Learn(Knowledge)` in AngelScript; the world context is implicit).
- Test it with the **Knows** condition: unlock dialogue topics, interaction offers, objectives.
- Knowledge whose prerequisites aren't known can't be learned yet; chain clues into deductions this way.

## 4. Settings

| Settings page | Setting | Meaning |
|---|---|---|
| *FlickerVoid > Quests* | Quests | Every quest the subsystem tracks (auto start, objectives, failure) |
| *FlickerVoid > Save* | Checkpoint Slot, Slot Index Name | Slot names |
| *FlickerVoid > Save* | Participants | Classes that add their own data to saves (section 5) |

## 5. Saving and loading

`UFVSaveSubsystem` (game instance subsystem): `Save To Slot`, `Load From Slot`, `Save Checkpoint`, `Load Checkpoint`, `Delete Slot`, `Get Slots` (metadata for a load menu), `Set Pending Thumbnail`, and `Add/Remove Save Block` (e.g. no saving during a chase).

A save holds: all facts (so quests, knowledge, reputation, titles, discoveries and time), Flow graph state, tracked quests and metadata.

Anything else (inventory, equipment, attribute base values, actor state) needs a **save participant**:
1. Subclass `UFVSaveParticipant` and override **Write Save** / **Read Save**. In AngelScript or Blueprint, store values in the save game's **Custom Data** map (name to string).
2. Add the class to *FlickerVoid > Save > Participants*; **Order** sets the sequence.

For actors, `UFVSaveableComponent` (FVFramework) gives each placed actor a stable Save Id and serializes its `SaveGame` properties (inventory items, equipment, attribute base values, lock state) with `WriteActorData` / `ReadActorData`. Those two are C++ only today and produce bytes, so a participant that saves actors this way has to be written in C++ (or the functions exposed to script first).

## 6. Editor

*Tools > Debug > FlickerVoid > Save Inspector* lists save slots and shows each one's metadata, facts and custom data.

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Story.Debug.HUD 1` | Active quests and their objectives on screen |
| `FV.Quest.Start <QuestIdOrAsset>` | Start a quest |
| `FV.Quest.Complete <QuestIdOrAsset>` | Complete a quest |
| `FV.Story.Learn <KnowledgeIdOrAsset>` | Learn knowledge |
