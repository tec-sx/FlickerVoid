# Intent-Driven Movement Handler Architecture

## Overview

The movement system has been refactored to be **pure intent-driven** with handlers as the execution layer.

## Architecture Flow

```
Player Input 
    ↓
[Character Request Methods]  ← ONLY manage intent tags
    ↓
Intent Tags Added/Removed  (Movement.Intent.*)
    ↓
[Movement Component] ← Monitors character tags every frame
    ↓
Finds handlers where ActivationTags match current character tags
    ↓
[Highest Priority Handler] ← Validates, grants state tags, executes action
    ↓
State Tags Updated (Movement.Gait.*, Movement.Stance.*, etc.)
    ↓
Animation queries state tags for blending
```

## Key Changes

### 1. Character Methods = Intent ONLY

**Before (BAD):**
```cpp
void RequestJump()
{
    if (CanJump())  // Character validates
    {
        Jump();     // Character executes
    }
}
```

**After (GOOD):**
```cpp
void RequestJump(bool bPressed)
{
    // Pure intent - just manage the tag
    if (bPressed)
        AddTag(Movement.Intent.Jump);
    else
        RemoveTag(Movement.Intent.Jump);
    
    // Handlers will validate and execute
}
```

### 2. Handlers = Validation + Execution

**Jump Handler Example:**
```angelscript
// Activation: Movement.Intent.Jump + Movement.Mode.OnGround
// Blocked By: Combat.State.Stunned, Movement.Restriction.NoJump
// Grants: Movement.Action.Jump
// Priority: Highest (100)

void OnEnter()
{
    // Execute the jump
    Character.Jump();
}

void GenerateMovement(float DeltaTime)
{
    // Monitor - if intent removed, do nothing
    if (!Character.HasTag(Movement.Intent.Jump))
    {
        // Intent cleared, handler will naturally exit
    }
}
```

**Traverse Handler Example:**
```angelscript
// Activation: Movement.Intent.Jump + Movement.Mode.OnGround
// Requires: ALSO checks for nearby traversable object
// Blocked By: Combat.State.Stunned
// Grants: Movement.Action.Traverse, Movement.Mode.Traversing
// Priority: Critical (255) - overrides jump

bool CanActivate(FGameplayTagContainer Tags)
{
    // Custom validation - is there something to traverse?
    return FindTraversableObject() != null;
}

void OnEnter()
{
    // Execute traverse
    StartTraverseAnimation();
    SetMovementMode(MOVE_Flying); // For traversal
}
```

### 3. No EFVMovementMode Enum

**Before (BAD):**
```cpp
// Handler info
EFVMovementMode MovementMode = EFVMovementMode::OnGround;

// Component
TMap<EFVMovementMode, Handler> MovementHandlers;
bool RequestMovementMode(EFVMovementMode Mode);
```

**After (GOOD):**
```cpp
// Handler info  
FGameplayTagContainer ActivationTags;  // [Movement.Mode.OnGround]

// Component
TArray<Handler> RegisteredHandlers;  // Iterable, tag-matched
bool TryActivate HandlerByTags(FGameplayTagContainer Tags);
```

**Benefits:**
- Add new handlers without enum changes
- Multiple handlers can respond to same tags (priority sorts)
- Handlers can have complex activation conditions (multiple tags)

### 4. Runtime Handler Registration

**Extensibility:**
```cpp
// Register custom handler at runtime (Blueprint/Angelscript)
FFVMovementHandlerInfo Info;
Info.ActivationTags.AddTag(Movement.Intent.Dash);
Info.ActivationTags.AddTag(Movement.Mode.OnGround);
Info.HandlerClass = BP_DashHandler;
Info.GrantedTags.AddTag(Movement.Action.Dash);
Info.Priority = 80;

MovementComponent->RegisterHandler(Info);

// Now dash works!
Character->AddTag(Movement.Intent.Dash);  // Handler responds
```

## Tag Categories

### Intent Tags (Set by character request methods)
- `Movement.Intent.Jump` - Player wants to jump
- `Movement.Intent.Crouch` - Player wants to crouch
- `Movement.Intent.Sprint` - Player wants to sprint
- `Movement.Intent.Jog` - Player wants to jog
- `Movement.Intent.Aim` - Player wants to aim
- `Movement.Intent.Interact` - Player wants to interact

### State Tags (Set by handlers when active)
- `Movement.Gait.Walking` - Currently walking
- `Movement.Gait.Jogging` - Currently jogging
- `Movement.Gait.Sprinting` - Currently sprinting
- `Movement.Stance.Crouching` - Currently crouched
- `Movement.Action.Jump` - Currently jumping
- `Movement.Action.Traverse` - Currently traversing

### Condition Tags (Auto-synced or set by systems)
- `Movement.Mode.OnGround` - On solid ground (auto)
- `Movement.Mode.InAir` - Airborne (auto)
- `Combat.State.Stunned` - Cannot act
- `Movement.Restriction.NoJump` - Jump disabled

## Handler Configuration Examples

### Ground Movement Handler
```
ActivationTags: [Movement.Mode.OnGround]
BlockedBy: [Combat.State.Stunned, Movement.State.Ragdoll]
Grants: [Movement.Gait.Walking]
Priority: Normal (50)
Description: "Basic ground movement - always active on ground"
```

### Sprint Handler
```
ActivationTags: [Movement.Intent.Sprint, Movement.Mode.OnGround]
BlockedBy: [Combat.State.Stunned, Movement.Stance.Crouching, Movement.Restriction.NoSprint]
Grants: [Movement.Gait.Sprinting]
Priority: High (75)
Description: "Sprint when player holds sprint input on ground"
```

### Jump Handler
```
ActivationTags: [Movement.Intent.Jump, Movement.Mode.OnGround]
BlockedBy: [Combat.State.Stunned, Movement.Restriction.NoJump]
Grants: [Movement.Action.Jump]
Priority: Highest (100)
Description: "Standard jump"
```

### Traverse/Mantle Handler
```
ActivationTags: [Movement.Intent.Jump, Movement.Mode.OnGround]
RequiredTags: [Near.Traversable.Object] (Set by trace system)
BlockedBy: [Combat.State.Stunned]
Grants: [Movement.Action.Traverse, Movement.Mode.Traversing]
Priority: Critical (255)
Description: "Mantle over obstacles - overrides jump when near traversable"
```

### Air Movement Handler
```
ActivationTags: [Movement.Mode.InAir]
BlockedBy: []
Grants: []
Priority: Normal (50)
Description: "Air control while falling"
```

## Benefits

### ✅ Pure Intent Character Class
- Character only manages intent tags
- No validation logic in character
- No direct CMC function calls (Jump(), Crouch(), etc.)
- Override-safe - scripts can't break execution flow

### ✅ Handlers Own Execution
- Handlers validate based on tags
- Handlers execute CMC functions
- Handlers grant state tags
- Handlers can be complex (Angelscript) or simple (C++)

### ✅ Infinite Extensibility
- Add new handlers without code changes
- Register handlers at runtime
- Multiple handlers can respond to same intent
- Priority system resolves conflicts

### ✅ No Enum Pollution
- No `EFVMovementMode` enum
- No switch statements
- No hardcoded mode mappings
- Tag-based matching is flexible

### ✅ Traverse = Just Another Jump Handler
- No special `TryTraverseAction()` method
- Traverse handler has higher priority than jump
- If traversable nearby, traverse activates
- If not, regular jump activates
- Same intent tag, different handlers

## Migration Example

### OLD - Hardcoded Logic
```cpp
void RequestJump()
{
    if (IsOnGround())
    {
        if (FindTraversableObject())
        {
            StartTraverse();
        }
        else
        {
            Jump();
        }
    }
}
```

### NEW - Intent + Handlers
```cpp
// Character
void RequestJump(bool bPressed)
{
    if (bPressed)
        AddTag(Movement.Intent.Jump);
    else
        RemoveTag(Movement.Intent.Jump);
}

// Traverse Handler (Angelscript)
// Activation: [Movement.Intent.Jump, Movement.Mode.OnGround]
// Priority: 255
bool CanActivate(Tags)
{
    return FindTraversableObject() != null;
}
void OnEnter()
{
    StartTraverse();
}

// Jump Handler (Angelscript)
// Activation: [Movement.Intent.Jump, Movement.Mode.OnGround]
// Priority: 100
void OnEnter()
{
    Character.Jump();
}

// Result: Same Intent tag, handlers compete, traverse wins if valid
```

## Implementation Status

✅ Intent tags added to taxonomy  
✅ Character methods refactored to pure intent  
✅ `TryTraverseAction()` removed from character  
✅ `FVMovementHandlerInfo` uses `ActivationTags` instead of enum  
✅ Movement component header updated to tag-based API  
✅ Movement component extensibility methods added  

🔄 Movement component implementation (next step)  
🔄 Example handlers in Angelscript (after CMC done)  

## Next Steps

1. Implement new CMC methods:
   - `TryActivateHandlerByTags()`
   - `UpdateActiveHandler()`
   - `RegisterHandler()` / `UnregisterHandler ByTags()`
   - `FindBestHandler()` (tag-based matching)

2. Update existing Angelscript handlers:
   - Add ActivationTags to handler config
   - Make handlers execute CMC functions
   - Remove validation from character

3. Create example handlers:
   - Ground movement handler
   - Jump handler
   - Traverse handler
   - Sprint handler
   - Crouch handler

4. Test handler priority resolution
5. Test runtime handler registration

---

**This is the proper intent-driven architecture with clean separation of concerns.**

