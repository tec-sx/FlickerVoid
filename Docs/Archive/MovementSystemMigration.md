# Migration Guide: Old to New Movement System

## Quick Start

### 1. Remove Old Handler Classes (C++)

Delete these files if they exist:
- `UFVEmptyMovementHandler` (.h and .cpp)
- `UFVGroundMovementHandler` (.h and .cpp)
- `UFVInAirMovementHandler` (.h and .cpp)
- `UFVTraverseMovementHandler` (.h and .cpp)

### 2. Create Movement Handler Data Asset

1. Content Browser → Right-click → Miscellaneous → Data Asset
2. Select **UFVMovementHandlerDataAsset**
3. Name it `DA_DefaultMovementConfig`
4. Save to `/Content/Data/Movement/`

### 3. Create Blueprint Handlers

For each movement mode, create a Blueprint:

#### Ground Movement (BP_GroundMovementHandler)

1. Create Blueprint Class → Parent: **FVMovementHandlerBase**
2. Name: `BP_GroundMovementHandler`

**Event On Enter:**
```blueprint
Get Movement Component
├─ Set Max Walk Speed: 600
├─ Set Max Acceleration: 2048
└─ Set Rotation Rate: (0, 360, 0)
```

**Event Generate Movement:**
```blueprint
Get Character
├─ Get Input Axis Value "MoveForward"
├─ Get Input Axis Value "MoveRight"
├─ Add Movement Input (Forward Direction, Forward Value)
└─ Add Movement Input (Right Direction, Right Value)
```

#### Air Movement (BP_AirMovementHandler)

1. Create Blueprint Class → Parent: **FVMovementHandlerBase**
2. Name: `BP_AirMovementHandler`

**Variables:**
- `JumpCount`: Integer = 0
- `MaxAirJumps`: Integer = 1

**Event On Enter:**
```blueprint
Get Character Velocity
├─ Break Vector (Z component)
├─ Branch: Z > 0?
│   YES: Set Jump Count = 0
│   NO: Set Jump Count = 1
└─ Get Movement Component → Set Air Control: 0.3
```

**Event Generate Movement:**
```blueprint
Get Character
├─ Get Input (Forward/Right)
├─ Add Movement Input (Direction, Air Control)
│
└─ Branch: Jump Pressed && Jump Count < Max Air Jumps?
    YES:
    ├─ Launch Character (0, 0, 600)
    └─ Increment Jump Count
```

**Event On Exit:**
```blueprint
Set Jump Count = 0
```

### 4. Configure Data Asset

Open `DA_DefaultMovementConfig`:

**Common Settings:**
```
Default Movement Speed: 600
Max Movement Speed: 1200
Acceleration Rate: 2048
Default Jump Velocity: 600
Default Gravity Scale: 1.0
Air Control: 0.05
```

**Movement Handlers (Add 2 entries):**

**Handler [0] - Ground Movement:**
```
Movement Mode: OnGround
Handler Class: BP_GroundMovementHandler
Priority: 50
Can Be Interrupted: true

Required Tags: (empty for basic setup)
Blocked By Tags:
  - Movement.State.Stunned

Granted Tags:
  - Movement.Mode.OnGround

Description: "Standard ground movement"
```

**Handler [1] - Air Movement:**
```
Movement Mode: InAir
Handler Class: BP_AirMovementHandler
Priority: 50
Can Be Interrupted: true

Required Tags: (empty for basic setup)
Blocked By Tags:
  - Movement.State.Stunned

Granted Tags:
  - Movement.Mode.InAir

Description: "Air movement with double jump"
```

### 5. Update Character Blueprint

1. Open your Character Blueprint
2. Select **Character Movement Component**
3. Find **Movement Configuration** section
4. Set **Movement Config** = `DA_DefaultMovementConfig`
5. Compile and Save

### 6. Test

1. Play in editor
2. Character should move using new system
3. Check Output Log for movement system initialization messages

## Code Changes Required

### Old Character Movement Code

**REMOVE:**
```cpp
// Old hardcoded handler initialization
void UFVCharacterMovementComponent::InitializeMovementHandlers()
{
    MovementModeHandlers.Add(EFVMovementMode::OnGround, NewObject<UFVGroundMovementHandler>());
    MovementModeHandlers.Add(EFVMovementMode::InAir, NewObject<UFVInAirMovementHandler>());
}
```

**REPLACE WITH:**
```cpp
// Now handled automatically from data asset
// Just assign MovementConfig in editor or constructor
```

### Character Construction

**Old:**
```cpp
// Constructor
UFVCharacterMovementComponent* MovementComp = GetCharacterMovement<UFVCharacterMovementComponent>();
// Handlers created in InitializeMovementHandlers()
```

**New:**
```cpp
// Constructor or defaults
UFVCharacterMovementComponent* MovementComp = GetCharacterMovement<UFVCharacterMovementComponent>();
MovementComp->MovementConfig = LoadObject<UFVMovementHandlerDataAsset>(
    nullptr, 
    TEXT("/Game/Data/Movement/DA_DefaultMovementConfig")
);
```

Or simply assign in Blueprint (recommended).

### Input Handling

**Old:**
```cpp
// Direct movement mode switching
if (Input->IsPressed("Jump"))
{
    MovementComponent->TransitionToMovementMode(EFVMovementMode::InAir);
}
```

**New (Tag-Based):**
```cpp
// Add tags, let system decide best handler
if (Input->IsPressed("Jump"))
{
    MovementComponent->AddMovementTag(FFVMovementTags::Get().Movement_Action_Jump);
}

// Or request specific mode
if (Input->IsPressed("Jump"))
{
    MovementComponent->RequestMovementMode(EFVMovementMode::InAir);
}
```

## Advanced Migration

### Sprint System

**Old (Hardcoded in Handler):**
```cpp
void UFVGroundMovementHandler::GenerateMovement(float DeltaTime)
{
    if (Input->IsPressed("Sprint"))
    {
        MovementComponent->MaxWalkSpeed = 900.0f;
    }
    else
    {
        MovementComponent->MaxWalkSpeed = 600.0f;
    }
}
```

**New (Separate Handler with Tags):**

1. Create `BP_SprintMovementHandler` (inherit from FVMovementHandlerBase)
2. In Data Asset, add new handler:

```
Movement Mode: OnGround
Handler Class: BP_SprintMovementHandler
Priority: 75 (higher than normal walking)

Required Tags:
  - Movement.Mode.OnGround
  - Movement.Action.Sprint (from input)

Granted Tags:
  - Movement.State.Sprinting

Custom Data:
  SprintSpeed: "900.0"
```

3. In input code:
```cpp
void AMyCharacter::SprintPressed()
{
    GetCharacterMovement<UFVCharacterMovementComponent>()
        ->AddMovementTag(FFVMovementTags::Get().Movement_Action_Sprint);
}

void AMyCharacter::SprintReleased()
{
    GetCharacterMovement<UFVCharacterMovementComponent>()
        ->RemoveMovementTag(FFVMovementTags::Get().Movement_Action_Sprint);
}
```

### Traversal System

**Old (Complex state machine):**
```cpp
if (CanClimb())
{
    MovementComponent->TransitionToMovementMode(EFVMovementMode::Traversing);
    CurrentTraversalType = ETraversalType::Climb;
}
else if (CanVault())
{
    MovementComponent->TransitionToMovementMode(EFVMovementMode::Traversing);
    CurrentTraversalType = ETraversalType::Vault;
}
```

**New (Multiple Handlers):**

Create separate handlers:
- `BP_ClimbMovementHandler`
- `BP_VaultMovementHandler`
- `BP_MantleMovementHandler`

Each with appropriate:
- Required Tags (e.g., `Movement.Surface.Climbable`)
- Priorities
- Activation logic

Detection code becomes:
```cpp
if (CanClimb())
{
    MovementComponent->AddMovementTag(FFVMovementTags::Get().Movement_Surface_Climbable);
    // System automatically activates climb handler
}
```

## Troubleshooting

### "Movement Config is null"

**Solution:** Assign the data asset to MovementConfig property in Character Blueprint

### "No handler found for mode"

**Solution:** Add the handler to the Movement Handlers array in data asset

### "Handler not activating"

**Solution:** 
1. Check Required Tags are present
2. Check priority vs current handler
3. Enable verbose logging: `Log LogFVMovement Verbose`

### Compile errors about old handler classes

**Solution:** Delete old handler .h and .cpp files, rebuild

### "Movement feels different"

**Solution:** Adjust Common Settings in data asset to match old values:
- Default Movement Speed
- Acceleration Rate
- Braking Deceleration
- Air Control
- etc.

## Benefits of New System

✅ **No C++ required** for new movement modes
✅ **Tag-based activation** - more flexible than state machines
✅ **Priority system** - automatic conflict resolution
✅ **Data-driven** - easy to tweak without recompiling
✅ **Cooldown/timing** support built-in
✅ **Network-ready** with replication flags
✅ **Better debugging** with comprehensive logging
✅ **GAS-like** architecture for consistency

## Next Steps

1. Complete basic migration above
2. Test all movement modes work
3. Add new handlers as Blueprints for new features
4. Gradually add tags for more complex activation rules
5. Configure priorities to resolve conflicts
6. Add cooldowns/timing constraints as needed

See `Docs/DataDrivenMovementSystem.md` for complete documentation.
