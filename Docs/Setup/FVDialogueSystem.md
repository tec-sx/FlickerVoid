# FVDialogueSystem setup

Conversations and barks built on FlowGraph. A conversation is a Flow asset made of Line, Choice and End Conversation nodes; the dialogue subsystem presents lines and choices to your UI and plays voices.

Modules: `FVDialogueSystem`, `FVDialogueSystemDebug`, `FVDialogueSystemEditor`. Requires Flow and FVFramework.

## 1. Quick start

1. Make character definitions for the speakers (FVFramework) and put an FV Identity Component on each speaker's Blueprint.
2. Make a conversation Flow asset (section 2).
3. Add **FV Dialogue Participant Component** to the NPC and set its **Dialogue** to that asset.
4. Make the dialogue widget (section 4).
5. Start the conversation: a Talk interaction offer (FVInteractionSystem) whose ability calls `Start Dialogue` (see `Script/Abilities/FVTalkAbility.as`), or call `Start Dialogue(Player)` on the participant component directly.

## 2. Conversation graphs

`FA_Dialogue_<Npc>_<Topic>`: a normal Flow asset (*Flow > Flow Asset*). Nodes:

| Node | Use |
|---|---|
| **Line** | **Speaker** (character definition; empty = the conversation owner, usually the NPC), **Text**, **Voice**, **Mood** (`Dialogue.Mood.*`), **Duration** (0 = from voice or text length), **Skippable**, **Wait For Input** |
| **Choice** | **Options**: text, **Conditions** (their failure presentation hides an option or shows it locked with a reason) and **On Chosen** effects. One output pin per option |
| **End Conversation** | Ends the conversation and finishes the graph. Use it instead of *Finish* in dialogue graphs |
| **Bark** | Plays one ambient line from a bark table (section 3) |

Mix in the generic nodes from FVFramework and other plugins: *FV Branch* (conditions), *FV Apply Effects* (set facts, learn knowledge, change standing, give items), *FV Wait For Conditions*, *Set Quest State*.

Example: `Line (NPC greets)` > `Choice [Ask about the docks | Show the badge (needs Is Wearing Police) | Leave]` > lines per branch > `End Conversation`.

Use *Set Dialogue* on the participant component (or an effect calling it from script) to swap an NPC's conversation as the story moves on.

## 3. Barks

| Asset | Class | Columns |
|---|---|---|
| `DT_Barks_<Group>` | Data table, row `FVBarkTableRow` | **Text**, **Voice**, **Display Duration**, **Identity Tag** (the speaker must own it), **Required Tags** / **Blocking Tags** (checked against speaker and player tags), **Conditions** |

Put a Bark node in a Flow graph owned by the NPC (e.g. an ambient graph on a Flow component or StateTree-driven flow). It picks a valid row, avoids repeating the last one for that speaker, broadcasts it and optionally waits for it to finish.

## 4. Dialogue UI

`WBP_Dialogue` (User Widget, pushed into a UI layer such as `UI.Layer.Game`):
1. Get `UFVDialogueSubsystem` (`UFVDialogueSubsystem::Get()` in AngelScript).
2. Bind **On Conversation Started** / **On Conversation Ended** to show and hide the widget.
3. Bind **On Line Started**: show speaker name (Speaker's Display name), text and mood.
4. Bind **On Choices Presented**: list the choices; locked ones have *Available* off and a *Locked Reason*.
5. On input call **Advance** (next line) or **Choose(Index)**.
6. Bind **On Bark** for floating ambient lines over the speaker.

## 5. Speakers and participants

**FV Dialogue Participant Component** on every actor that talks:
- **Dialogue**: the Flow asset started by *Start Dialogue*.
- Events **On Line Started**, **On Line Finished** and **On Bark** fire on the speaking actor, so its Blueprint can play animations, lip sync or look-at.
- Speakers resolve through identity: the instigator or owner when their definition matches, otherwise any registered actor with that character definition.

## 6. Settings

*FlickerVoid > Dialogue*:

| Setting | Meaning |
|---|---|
| Min Line Duration, Seconds Per Character | Duration of an unvoiced line: max(min, characters x per-character) |
| Voice Padding | Extra time after a voiced line ends |

## 7. Gameplay tags

`Dialogue.Mood` is native; create the moods you need under it (`Dialogue.Mood.Angry`, `.Calm`, ...).

## 8. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Dialogue.Debug.HUD 1` | The running conversation, speaker and choices on screen |
| `FV.Dialogue.Advance` | Skip to the next line |
| `FV.Dialogue.Choose <Index>` | Pick a choice |
| `FV.Dialogue.End` | Abort the conversation |
