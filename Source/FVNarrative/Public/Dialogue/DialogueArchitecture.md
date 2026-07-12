# Dialogue Subsystem — Architecture Reference
## UE5.7 | C++ + Angelscript | Single Player

---

## 1. System Layers

```
┌────────────────────────────────────────────────────────────────────┐
│                        GAME WORLD                                  │
│  NPC Sight Radius ──► BroadcastSignal("Signal.Greeting", Context) │
└────────────────────────────┬───────────────────────────────────────┘
                             │
                             ▼
┌────────────────────────────────────────────────────────────────────┐
│                    UDialogueSubsystem (GameInstance)               │
│                                                                    │
│  1. Receives FDialogueSignal (Tag + Context snapshot)             │
│  2. Queries UDialogueDatabaseSubsystem for matching rows          │
│  3. Runs UDialogueConditionEvaluator → filters + scores lines     │
│  4. Selects best FDialogueLineRow                                 │
│  5. Spawns / feeds UDialogueStateTreeComponent                    │
│  6. Broadcasts OnLineReady delegate to UI                         │
└────────────┬───────────────────────────┬───────────────────────────┘
             │                           │
             ▼                           ▼
┌────────────────────┐       ┌────────────────────────────────────┐
│ UDialogue          │       │  UDialogueDatabaseSubsystem        │
│ StateTreeComponent │       │                                    │
│                    │       │  Caches & indexes all DataTables   │
│  State = one node  │       │  by SignalTag + SpeakerID          │
│  in the dialogue   │       │  (built at game start)             │
│  graph             │       └────────────────────────────────────┘
│                    │
│  Tasks:            │       ┌────────────────────────────────────┐
│  - PlayLine        │       │  UDialogueContextBuilder           │
│  - WaitForChoice   │       │                                    │
│  - EvalCondition   │       │  Snapshots world/character state   │
│  - TriggerEvent    │       │  into FDialogueContext at signal   │
│                    │       │  time. Immutable during eval.      │
└────────────────────┘       └────────────────────────────────────┘
```

---

## 2. Data Table Schema

Two data tables drive the entire system:

### `DT_DialogueLines`  (row type: `FDialogueLineRow`)
Each row = one candidate spoken line from an NPC (or player).

| Field | Type | Purpose |
|---|---|---|
| LineID | FName (row key) | Unique identifier |
| SpeakerID | FGameplayTag | `Speaker.NPC.Deale` |
| SignalTag | FGameplayTag | `Signal.Greeting` |
| LocalizationKey | FText | Actual spoken/displayed text |
| VoiceAsset | TSoftObjectPtr<USoundBase> | Optional audio |
| Conditions | FDialogueConditionSet | All filter logic |
| Priority | int32 | Tie-break (higher wins) |
| Weight | float | Among equal-priority, weighted random |
| bIsRepeatable | bool | False = consumed after first play |
| bConsumeOnPlay | bool | One-shot (key item exchange etc.) |
| ResponseOptionIDs | TArray<FName> | Row keys into DT_DialogueResponses |
| NextLineID | FName | Auto-advance (no player choice) |
| EventOnPlay | FGameplayTag | Optional: fires a game event |

### `DT_DialogueResponses`  (row type: `FDialogueResponseRow`)
Each row = one player reply option.

| Field | Type | Purpose |
|---|---|---|
| ResponseID | FName (row key) | Unique identifier |
| DisplayText | FText | Shown in choice UI |
| Conditions | FDialogueConditionSet | Can be hidden/greyed if failed |
| NextLineID | FName | Which NPC line plays next |
| NextSignalTag | FGameplayTag | OR: re-triggers signal selection |
| EventOnChoose | FGameplayTag | Optional: fires a game event |

---

## 3. Condition System

`FDialogueConditionSet` composes multiple condition types.
Each evaluates against `FDialogueContext` (a read-only snapshot).

### Condition Types

| Condition | Hard/Soft | Example |
|---|---|---|
| `FTagRequirement` | Hard | NPC must have `Personality.Hostile` |
| `FTagBlocker` | Hard | World must NOT have `World.Time.Night` |
| `FAttributeRangeCondition` | Hard/Soft | Notoriety >= 50 |
| `FRelationshipCondition` | Hard/Soft | Relationship [2, 5] |
| `FFactionStandingCondition` | Hard/Soft | Faction.Merchants >= Neutral |
| `FQuestStateCondition` | Hard | Quest.MainQuest.Act2 is Active |
| `FWorldStateCondition` | Hard | WorldTag `World.Weather.Rain` present |
| `FFirstMeetCondition` | Hard | Relationship == 0 (first encounter) |

### Scoring
After hard conditions pass, each soft condition adds to a **specificity score**:
- Base: +10 per matched soft condition
- Weighted by field: Relationship match +15, Quest match +20, Personality +10
- A line with 3 specific conditions beating an "always valid" fallback line

---

## 4. Context Snapshot (`FDialogueContext`)

Built once per signal, immutable during evaluation.

```
FDialogueContext {
    InstigatorActor       (Player pawn ref)
    TargetActor           (NPC ref)
    SignalTag             (what triggered this)
    SpeakerID             (NPC tag)
    WorldStateTags        (Day/Night, Weather, Location, etc.)
    InstigatorTags        (Player gameplay tags)
    TargetTags            (NPC personality, state tags)
    RelationshipValue     (int32, 0=stranger ... 10=bond)
    NotorietyValue        (float 0..100)
    FactionStandings      (TMap<FGameplayTag, EFactionStanding>)
    ActiveQuestTags       (all currently active/complete quest tags)
    ConsumedLineIDs       (set of already-played one-shot lines)
    Timestamp             (FDateTime, for time-of-day logic)
}
```

---

## 5. State Tree Flow

Each dialogue session (from signal to session end) runs a **State Tree instance**:

```
[Root]
  ├─ [SelectLine]          ← Enter: query DB, pick best line
  │    Task: FDialogueSelectLineTask
  │    On Success ──► [PlayLine]
  │    On Failure ──► [Fallback] or [End]
  │
  ├─ [PlayLine]            ← Display text, play audio
  │    Task: FDialoguePlayLineTask
  │    Evaluator: FDialogueContextEvaluator (live world checks)
  │    On Complete:
  │      if ResponseOptions.Num() > 0 ──► [WaitForPlayerChoice]
  │      if NextLineID valid           ──► [SelectLine] (with NextLineID)
  │      else                          ──► [End]
  │
  ├─ [WaitForPlayerChoice] ← Present UI, wait for input
  │    Task: FDialogueWaitChoiceTask
  │    On Choose ──► [ProcessChoice]
  │
  ├─ [ProcessChoice]       ← Fire EventOnChoose, update relationship etc.
  │    Task: FDialogueProcessChoiceTask
  │    On Complete ──► [SelectLine] (NextLineID or NextSignalTag)
  │
  └─ [End]                 ← Tear down session, notify subsystem
       Task: FDialogueEndTask
```

---

## 6. Signal Flow (End-to-End)

```
1. NPC radius overlap detected (C++ or AS component)
2. NPC calls: GEngine->GetEngineSubsystem<UDialogueSubsystem>()
              ->BroadcastSignal(SignalTag, InstigatorActor, TargetActor)
3. UDialogueSubsystem:
   a. UDialogueContextBuilder::Build(Signal, Instigator, Target) → FDialogueContext
   b. UDialogueDatabaseSubsystem::QueryLines(SignalTag, SpeakerID) → TArray<FDialogueLineRow*>
   c. UDialogueConditionEvaluator::FilterAndScore(Lines, Context) → Best FDialogueLineRow*
   d. CreateDialogueSession(BestLine, Context) → UDialogueSession
   e. UDialogueSession::StartStateTree()
4. State Tree tasks drive the conversation
5. UI listens to delegates on UDialogueSubsystem
6. Session ends → UDialogueSubsystem::OnSessionEnded()
```

---

## 7. Angelscript Integration Points

Angelscript hooks in at:
- Custom **State Tree Tasks** (derive from `FDialogueBaseTask`, implement in AS)
- **Condition evaluators** (register custom condition types from AS)
- **Signal broadcasters** on NPC/player components (all in AS)
- **UI binding** to subsystem delegates (all in AS widget controllers)
- **Blueprint Data Assets** for per-NPC configuration (which DT rows belong to whom)

---

## 8. File Map

```
DialogueSubsystem/
├── Public/
│   ├── Dialogue/
│   │   ├── DialogueSubsystem.h           ← GameInstance subsystem (main entry)
│   │   ├── DialogueSession.h             ← One active conversation
│   │   ├── DialogueContext.h             ← Immutable snapshot struct
│   │   ├── DialogueDatabaseSubsystem.h   ← DataTable cache & query
│   │   └── DialogueContextBuilder.h      ← Builds FDialogueContext
│   ├── Conditions/
│   │   ├── DialogueConditionTypes.h      ← All FDialogueCondition structs
│   │   ├── DialogueConditionSet.h        ← Composites + evaluator interface
│   │   └── DialogueConditionEvaluator.h  ← Filter + score pipeline
│   ├── StateTree/
│   │   ├── DialogueStateTreeTasks.h      ← All ST task structs
│   │   └── DialogueStateTreeEvaluators.h ← ST evaluator structs
│   └── Data/
│       ├── DialogueLineRow.h             ← DT_DialogueLines row type
│       └── DialogueResponseRow.h         ← DT_DialogueResponses row type
├── Private/
│   ├── Dialogue/
│   │   ├── DialogueSubsystem.cpp
│   │   ├── DialogueSession.cpp
│   │   ├── DialogueDatabaseSubsystem.cpp
│   │   └── DialogueContextBuilder.cpp
│   ├── Conditions/
│   │   └── DialogueConditionEvaluator.cpp
│   └── StateTree/
│       ├── DialogueStateTreeTasks.cpp
│       └── DialogueStateTreeEvaluators.cpp
└── Angelscript/
    ├── DialogueSignalComponent.as         ← NPC/Player signal broadcaster
    ├── DialogueWidgetController.as        ← UI binding to subsystem delegates
    └── DialogueNPCComponent.as            ← NPC-side context contribution
```