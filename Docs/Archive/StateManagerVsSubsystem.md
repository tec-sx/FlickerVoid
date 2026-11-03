# UFVCharacterStateManager - Architecture Explanation

## Brief Overview

**What it is:** A per-character state management object that stores ALL character state as gameplay tags.

**What it does:**
1. **Stores** a single `FGameplayTagContainer Tags` - the source of truth
2. **Auto-syncs** tags from CMC (movement mode, crouch, etc.) every frame
3. **Validates** manual tag additions/removals
4. **Broadcasts** events when tags change (for animation, UI, abilities)

**Flow:**
```
Character Action → StateManager.AddTag() → Tags Container Updated 
→ OnTagChanged Event Fired → Animation/UI React
```

---

## Why "Manager" not "Subsystem"?

### Unreal's USubsystem Classes

USubsystems are **SINGLETONS** with specific scopes:

| Class | Scope | Lifetime | Purpose |
|-------|-------|----------|---------|
| `UEngineSubsystem` | **One per Engine** | Entire editor/game session | Cross-game global functionality |
| `UGameInstanceSubsystem` | **One per GameInstance** | Game session | Persistent across level loads |
| `UWorldSubsystem` | **One per World** | Current level | Level-specific global functionality |
| `ULocalPlayerSubsystem` | **One per Local Player** | Player session | Per-player (splitscreen) functionality |

**Key Point:** They're **automatically created** by the engine and are **globally accessible** within their scope.

**Examples:**
```cpp
// Get singleton subsystem
UMyGameInstanceSubsystem* Subsystem = GameInstance->GetSubsystem<UMyGameInstanceSubsystem>();

// Everyone gets the SAME instance - it's a singleton!
UMyWorldSubsystem* WorldSub = GetWorld()->GetSubsystem<UMyWorldSubsystem>();
```

### Our StateManager

**Scope:** **One per CHARACTER** (NOT singleton!)

| Property | USubsystem | UFVCharacterStateManager |
|----------|------------|--------------------------|
| **Instances** | One (singleton) | Many (one per character) |
| **Lifetime** | Managed by engine | Owned by character |
| **Creation** | Automatic | Manual (`CreateDefaultSubobject`) |
| **Access** | Global getter | Through owning character |
| **Purpose** | Shared functionality | Per-instance state |

**Example:**
```cpp
// Each character has ITS OWN state manager
AFVCharacter* Player = SpawnPlayer();
Player->GetStateManager()->AddTag(...);  // Player's state

AFVCharacter* Enemy1 = SpawnEnemy();
Enemy1->GetStateManager()->AddTag(...);  // Enemy1's state (different!)

AFVCharacter* Enemy2 = SpawnEnemy();
Enemy2->GetStateManager()->AddTag(...);  // Enemy2's state (also different!)

// 3 characters = 3 separate StateManager instances
```

---

## Correct Usage Pattern

### ✅ RIGHT - Character-Owned State Manager

```cpp
UCLASS()
class AFVCharacter : public ACharacter
{
    UPROPERTY()
    TObjectPtr<UFVCharacterStateManager> StateManager;  // Each character owns one
};

// Usage
Character1->GetStateManager()->AddTag(...);  // Character 1's state
Character2->GetStateManager()->AddTag(...);  // Character 2's state (independent)
```

### ❌ WRONG - If We Used UWorldSubsystem

```cpp
// Hypothetical WRONG approach
UCLASS()
class UCharacterStateSubsystem : public UWorldSubsystem  // Singleton!
{
    // This would be SHARED by ALL characters - BAD!
    FGameplayTagContainer AllCharacterTags;  // Whose tags are these??
};

Character1->AddTag(...);  // Where does this go?
Character2->AddTag(...);  // How do we separate state??
// Can't work - need per-character state!
```

---

## When To Use Real USubsystems

**Use USubsystems for:**
- Global managers (spawn system, save system, analytics)
- Shared services (audio manager, UI manager)
- Cross-character systems (quest system, faction system)
- Persistent data (game settings, player progression)

**Examples:**

### UGameInstanceSubsystem - Save System
```cpp
UCLASS()
class USaveGameSubsystem : public UGameInstanceSubsystem
{
    // ONE save system for entire game
    void SaveGame();
    void LoadGame();
};

// Any character can access THE save system
GetGameInstance()->GetSubsystem<USaveGameSubsystem>()->SaveGame();
```

### UWorldSubsystem - Quest System
```cpp
UCLASS()
class UQuestSubsystem : public UWorldSubsystem
{
    TArray<FQuest> ActiveQuests;  // Shared by all characters in world
    
    void CompleteQuest(FName QuestID);
};

// Global quest state, affects all characters
GetWorld()->GetSubsystem<UQuestSubsystem>()->CompleteQuest("Main_Quest_01");
```

### DON'T Use USubsystem For - Character State
```cpp
// ❌ WRONG - Character state is PER-CHARACTER, not global!
UCLASS()
class UCharacterStateSubsystem : public UWorldSubsystem  // BAD IDEA!
{
    // This would need a map of character -> state
    TMap<AFVCharacter*, FGameplayTagContainer> CharacterStates;  // Overcomplicated!
};

// Much simpler: Just put state IN the character!
UCLASS()
class AFVCharacter
{
    TObjectPtr<UFVCharacterStateManager> StateManager;  // ✅ CORRECT
};
```

---

## StateManager Architecture Details

### Ownership
```cpp
AFVCharacter::AFVCharacter()
{
    // Character creates and owns the manager
    StateManager = CreateDefaultSubobject<UFVCharacterStateManager>(TEXT("StateManager"));
}

void AFVCharacter::PostInitializeComponents()
{
    // Initialize with back-reference
    StateManager->Initialize(this);
}
```

### Lifetime
- **Created:** When character spawns
- **Lives:** As long as character exists
- **Destroyed:** When character is destroyed

### Memory
- One `FGameplayTagContainer` per character (~24 bytes + tag data)
- Negligible overhead (~200 bytes per character total)
- Much more efficient than global map approach

---

## Summary

| Question | Answer |
|----------|--------|
| **Is it a USubsystem?** | No - it's a per-character UObject |
| **Why "Manager"?** | Manages state for ONE character |
| **Why not USubsystem?** | USubsystems are singletons, we need per-instance |
| **When use USubsystem?** | For global/shared services |
| **When use Manager pattern?** | For per-actor/per-instance state |

**The Rename:**
- `UFVCharacterStateSubsystem` ❌ (misleading)
- `UFVCharacterStateManager` ✅ (accurate)

---

## Related Patterns in Unreal

### Per-Actor State Management (Our Pattern)
```cpp
UActorComponent          // Built-in per-actor functionality
UFVCharacterStateManager // Our custom per-character state
```

### Global Services (USubsystem Pattern)
```cpp
UGameInstanceSubsystem  // Game-wide services
UWorldSubsystem         // World-wide services
ULocalPlayer Subsystem   // Per-player services
```

### Comparison Table

| Need | Solution | Example |
|------|----------|---------|
| Per-character state | Character-owned UObject | `UFVCharacterStateManager` |
| Per-character functionality | `UActorComponent` | `UCharacterMovementComponent` |
| Global game service | `UGameInstanceSubsystem` | `USaveGameSubsystem` |
| Per-level service | `UWorldSubsystem` | `UQuestSubsystem` |
| Per-player service | `ULocalPlayerSubsystem` | `USettingsSubsystem` |

---

**Renamed to `UFVCharacterStateManager` for clarity and correctness!**

