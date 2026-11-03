# Jump System Refactor - Intent Separation

## What Changed

### Method Signatures

**Before:**
```cpp
void RequestJumpOrTraverse(bool bTrigger);
```

**After:**
```cpp
void RequestJump();              // No parameters
void RequestTraversalCheck();    // New method
```

---

## The New System

### 1. RequestJump() - Initial Jump Intent

**Called From:** `Input_JumpStarted` (Jump button pressed)

**What it does:**
```cpp
void AFVCharacter::RequestJump_Implementation()
{
    AddTag(Movement.Intent.Jump);
    
    // JumpHandler checks: on ground? not stunned?
    // TraverseHandler checks: traversable nearby? (higher priority)
}
```

**Tag Added:** `Movement.Intent.Jump`

**Handler Response:**
- **On Ground + Near Traversable:** TraverseHandler activates (Priority 255)
  - Starts mantle/vault animation
  - Adds `Movement.Action.Traverse` tag
  
- **On Ground + No Traversable:** JumpHandler activates (Priority 100)
  - Calls `Character->Jump()`
  - Adds `Movement.Action.Jump` tag

---

### 2. RequestTraversalCheck() - Continuous Air Check

**Called From:** `Input_JumpTriggered` (Jump button held, triggers continuously)

**What it does:**
```cpp
void AFVCharacter::RequestTraversalCheck_Implementation()
{
    AddTag(Movement.Intent.Traverse);
    
    // For mid-air traversal (e.g., grabbing ledges while jumping)
    // TraverseHandler checks for nearby climbable objects
}
```

**Tag Added:** `Movement.Intent.Traverse`

**Use Case:** Climbing/grabbing ledges mid-jump
- Player jumps toward wall
- While jump button held and in air, continuously checks for climbable surfaces
- If found, AirTraverseHandler can grab ledge

**Handler Response:**
- **In Air + Near Ledge:** AirTraverseHandler activates
  - Snaps to ledge
  - Starts climb animation
  - Adds `Movement.Action.Climb` tag

---

## Input Flow

```
Player presses Jump button
    ↓
Input_JumpStarted()
    ↓
Character->RequestJump()
    ↓
AddTag(Movement.Intent.Jump)
    ↓
CMC finds matching handler:
    - TraverseHandler (Priority 255) - if traversable nearby
    - JumpHandler (Priority 100) - otherwise
    ↓
Handler executes (Jump() or StartTraverse())


Player holds Jump button (continuous)
    ↓
Input_JumpTriggered() (fires every frame while held)
    ↓
Character->RequestTraversalCheck()
    ↓
AddTag(Movement.Intent.Traverse)
    ↓
AirTraverseHandler checks for ledges
    ↓
If ledge found: Grab and climb
```

---

## Enhanced Input Configuration

```cpp
// In FVPlayerController::InitializeInput()

// Jump Started - Initial jump intent
FVIC->BindNativeAction(
    InputConfig, 
    FVCoreTags::InputTag_Jump, 
    ETriggerEvent::Started,      // Button pressed
    this, 
    &ThisClass::Input_JumpStarted, 
    false
);

// Jump Triggered - Continuous traversal check
FVIC->BindNativeAction(
    InputConfig, 
    FVCoreTags::InputTag_Jump, 
    ETriggerEvent::Triggered,    // Button held (continuous)
    this, 
    &ThisClass::Input_JumpTriggered, 
    false
);
```

---

## Handler Examples

### Ground Jump Handler
```angelscript
// GroundJumpHandler.as
// ActivationTags: [Movement.Intent.Jump, Movement.Mode.OnGround]
// Priority: 100

void OnEnter()
{
    Character.Jump();
    Character.AddTag(Movement.Action.Jump);
}
```

### Ground Traverse Handler
```angelscript
// GroundTraverseHandler.as
// ActivationTags: [Movement.Intent.Jump, Movement.Mode.OnGround]
// Priority: 255 (Higher than jump!)

bool CanActivate(Tags)
{
    // Custom check
    return FindTraversableObjectInFront() != null;
}

void OnEnter()
{
    StartMantle();
    Character.AddTag(Movement.Action.Traverse);
}
```

### Air Ledge Grab Handler
```angelscript
// AirLedgeGrabHandler.as
// ActivationTags: [Movement.Intent.Traverse, Movement.Mode.InAir]
// Priority: 200

bool CanActivate(Tags)
{
    // Check for ledges while in air
    return FindLedgeInReach() != null;
}

void OnEnter()
{
    FVector LedgePos = FindLedgeInReach().Location;
    Character.SetActorLocation(LedgePos);
    PlayClimbAnimation();
    Character.AddTag(Movement.Action.Climb);
}

void OnExit()
{
    // Clean up the traverse intent
    Character.RemoveTag(Movement.Intent.Traverse);
}
```

---

## Tag Taxonomy

### Intent Tags (Set by character)
- `Movement.Intent.Jump` - Player wants to jump/traverse
- `Movement.Intent.Traverse` - Continuous check for traversal opportunities

### State Tags (Set by handlers)
- `Movement.Action.Jump` - Currently jumping
- `Movement.Action.Traverse` - Currently traversing (mantle/vault)
- `Movement.Action.Climb` - Currently climbing

### Mode Tags (Auto-synced)
- `Movement.Mode.OnGround` - On solid ground
- `Movement.Mode.InAir` - Airborne

---

## Benefits

### ✅ Clear Intent Separation
```cpp
// Jump intent (one-shot)
RequestJump();

// Traversal check (continuous)
RequestTraversalCheck();
```

### ✅ No Boolean Parameters
```cpp
// ❌ OLD - Confusing
RequestJumpOrTraverse(true);   // What does true mean?
RequestJumpOrTraverse(false);  // What does false mean?

// ✅ NEW - Clear
RequestJump();           // Jump!
RequestTraversalCheck(); // Check for traversal!
```

### ✅ Handlers Control Everything
```cpp
// Character doesn't care about ground checks, traversable objects, etc.
// Just sets intent tags - handlers validate and execute
```

### ✅ Air Traversal Support
```cpp
// Player can grab ledges mid-jump
// Continuously checks while jump button held
// AirTraverseHandler responds when ledge found
```

### ✅ Extensible
```cpp
// Add WallRunHandler that responds to Movement.Intent.Traverse + near wall
// Add SwingHandler that responds to Movement.Intent.Traverse + near rope
// No character code changes!
```

---

## Migration Notes

### PlayerController Changes
```cpp
// OLD
void Input_JumpStarted()
{
    Character->RequestJumpOrTraverse(false);
}

void Input_JumpTriggered()
{
    Character->RequestJumpOrTraverse(true);
}

// NEW
void Input_JumpStarted()
{
    Character->RequestJump();
}

void Input_JumpTriggered()
{
    Character->RequestTraversalCheck();
}
```

### Character Changes
- ✅ `RequestJumpOrTraverse(bool)` → `RequestJump()` + `RequestTraversalCheck()`
- ✅ No parameters
- ✅ Two separate intent tags

### New Tags
- ✅ `Movement.Intent.Jump` (existing)
- ✅ `Movement.Intent.Traverse` (NEW)

---

## Example Scenario

**Player runs toward wall and presses jump:**

1. `Input_JumpStarted()` → `RequestJump()`
2. Adds `Movement.Intent.Jump`
3. Character has tags: `[Intent.Jump, Mode.OnGround]`
4. GroundTraverseHandler checks: "Is there a wall nearby?"
   - **Yes:** GroundTraverseHandler activates (Priority 255)
     - Starts mantle animation
     - Character climbs wall
   - **No:** GroundJumpHandler activates (Priority 100)
     - Character jumps normally
5. If player keeps holding jump button:
   - `Input_JumpTriggered()` fires continuously
   - `RequestTraversalCheck()` called each frame
   - Adds `Movement.Intent.Traverse`
   - AirLedgeGrabHandler checks: "Is there a ledge nearby?"
     - **Yes:** Grabs ledge and climbs
     - **No:** Continues falling

**Result:** Smooth, contextual traversal with no complex logic in character class!

---

## Summary

**Before:** One confusing method with boolean parameter  
**After:** Two clear methods with distinct purposes

**Before:** Manual checks in character class  
**After:** Handlers validate and execute everything

**Before:** Limited to ground traversal  
**After:** Supports ground + air traversal

**Before:** Hard to extend  
**After:** Just add new handlers responding to same tags

**This is clean, intent-driven architecture!** 🎉

