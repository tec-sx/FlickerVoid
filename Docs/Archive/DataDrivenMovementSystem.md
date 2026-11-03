# Data-Driven Movement System Documentation

## Overview

The FlickerVoid movement system has been refactored to use a data-driven approach with gameplay tag-based activation, similar to the Gameplay Ability System (GAS). Movement handlers are now defined in Blueprint/Script rather than C++, providing maximum flexibility for designers and scripters.

## Architecture

### Core Components

1. **UFVMovementHandlerDataAsset** - Configuration data asset
   - Defines all movement handlers and their properties
   - Sets common movement parameters
   - Configures tag requirements and blocking rules

2. **UFVMovementHandlerBase** - Base class for handlers (Blueprint-friendly)
   - Designed for Blueprint/Script implementation
   - Provides lifecycle hooks (Initialize, Enter, GenerateMovement, Exit)
   - Manages state, cooldowns, and activation checks

3. **UFVCharacterMovementComponent** - Movement component
   - Reads configuration from data asset
   - Manages handler lifecycle and transitions
   - Handles gameplay tag-based activation
   - Automatic tag management

4. **FFVMovementTags** - Gameplay tag definitions
   - Centralized tag definitions in FVCore
   - Organized by category (Mode, State, Action, Restriction, Surface, Network)

## Creating a Movement Handler Data Asset

### Step 1: Create the Data Asset

1. In Content Browser, right-click → **Miscellaneous** → **Data Asset**
2. Select **UFVMovementHandlerDataAsset**
3. Name it (e.g., `DA_DefaultMovementConfig`)

### Step 2: Configure Common Settings

```
Default Movement Speed: 600.0
Max Movement Speed: 1200.0
Acceleration Rate: 2048.0
Default Jump Velocity: 600.0
Default Gravity Scale: 1.0
Air Control: 0.05
```

### Step 3: Add Movement Handlers

Click **+** on Movement Handlers array for each handler you need:

#### Example: Ground Movement Handler

```
Movement Mode: OnGround
Handler Class: BP_GroundMovementHandler (your Blueprint)

Required Tags:
  - Movement.Surface.Walkable (optional, for surface detection)

Blocked By Tags:
  - Movement.State.Stunned
  - Movement.State.Rooted

Granted Tags:
  - Movement.Mode.OnGround

Blocking Tags:
  - Movement.Mode.InAir
  - Movement.Mode.Traversing

Priority: 50 (Normal)
Can Be Interrupted: true
Minimum Active Time: 0.0
Cooldown Time: 0.0
Maximum Active Time: 0.0 (unlimited)

Replicates: true
Requires Authority: false
Allow Client Prediction: true

Description: "Standard ground movement with walking and running"
```

#### Example: Sprint Handler

```
Movement Mode: OnGround (can share mode with normal walking)
Handler Class: BP_SprintMovementHandler

Required Tags:
  - Movement.Mode.OnGround
  - Movement.Action.Sprint (from input)

Blocked By Tags:
  - Movement.State.Stunned
  - Movement.Restriction.NoSprint

Granted Tags:
  - Movement.State.Sprinting

Priority: 75 (High) - Will override normal walking

Can Be Interrupted: true
Minimum Active Time: 0.1
Cooldown Time: 0.5
Maximum Active Time: 10.0 (stamina limit)

Custom Data:
  SprintSpeedMultiplier: "2.0"
  StaminaCostPerSecond: "10.0"
```

#### Example: Dodge/Roll Handler

```
Movement Mode: OnGround
Handler Class: BP_DodgeMovementHandler

Required Tags:
  - Movement.Mode.OnGround

Blocked By Tags:
  - Movement.State.Stunned
  - Movement.State.Dodging (prevent double dodge)

Granted Tags:
  - Movement.State.Dodging

Blocking Tags:
  - Movement.Action.Jump
  - Movement.State.Sprinting

Priority: 100 (Highest) - Cannot be interrupted

Can Be Interrupted: false
Minimum Active Time: 0.8
Cooldown Time: 2.0
Maximum Active Time: 0.8

Consume Input: true

Custom Data:
  DodgeSpeed: "1200.0"
  InvulnerabilityFrames: "0.3"
```

### Step 4: Configure Transition Rules

```
Minimum Transition Time: 0.05 (prevent rapid switching)
Blend Handler Transitions: false
```

### Step 5: Set Default Tags

```
Default Tags:
  - Movement.Surface.Walkable
```

## Creating a Movement Handler Blueprint

### Step 1: Create Blueprint Class

1. Right-click in Content Browser → **Blueprint Class**
2. Search for **FVMovementHandlerBase**
3. Select it and name your Blueprint (e.g., `BP_GroundMovementHandler`)

### Step 2: Implement Handler Logic

#### Override Event: On Initialize

```blueprint
Event On Initialize
  ├─ Store Character reference
  ├─ Store Movement Component reference
  ├─ Cache any custom data from Config
  └─ Initialize state variables
```

#### Override Event: On Enter

```blueprint
Event On Enter
  ├─ Set movement parameters (speed, acceleration, etc.)
  ├─ Play enter animation/effects
  ├─ Apply any buffs/multipliers
  └─ Initialize handler state
```

#### Override Event: Generate Movement

```blueprint
Event Generate Movement (Delta Time)
  ├─ Get input from character
  ├─ Calculate desired movement direction
  ├─ Apply movement forces
  ├─ Update rotation
  ├─ Handle special logic (stamina, effects, etc.)
  └─ Check for exit conditions
```

#### Override Event: On Exit

```blueprint
Event On Exit
  ├─ Restore default movement parameters
  ├─ Play exit animation/effects
  ├─ Clean up state
  └─ Remove temporary buffs
```

#### Optional: Can Activate

```blueprint
Event Can Activate (Current Tags) → Return Value
  ├─ Check stamina
  ├─ Check cooldowns
  ├─ Check custom conditions
  └─ Return true/false
```

### Example: Ground Movement Handler

```blueprint
// Variables
- GroundSpeed: Float = 600.0
- SprintMultiplier: Float = 1.5
- IsSprinting: Bool = false

// On Initialize
Event On Initialize
  └─ (Store references as needed)

// On Enter
Event On Enter
  ├─ Get Movement Component
  ├─ Set Max Walk Speed = Ground Speed
  └─ Set Rotation Rate = (0, 360, 0)

// Generate Movement
Event Generate Movement (Delta Time)
  ├─ Get Character
  ├─ Get Input Vector (Forward/Right)
  │
  ├─ Branch: Has Input?
  │   YES:
  │   ├─ Calculate Movement Direction (Forward * Input.X + Right * Input.Y)
  │   ├─ Add Movement Input (Direction, 1.0)
  │   │
  │   ├─ Check Sprint Input
  │   │   └─ Set IsSprinting
  │   │
  │   └─ Branch: Is Sprinting?
  │       YES: Set Max Walk Speed = Ground Speed * Sprint Multiplier
  │       NO: Set Max Walk Speed = Ground Speed
  │
  └─ (Optional) Check for jump input, apply forces, etc.

// On Exit
Event On Exit
  └─ Reset Is Sprinting = false
```

### Example: Air Movement Handler

```blueprint
// Variables
- AirControl: Float = 0.3
- JumpCount: Int = 0
- MaxAirJumps: Int = 1

// On Enter
Event On Enter
  ├─ Get Current Velocity Z
  ├─ Branch: Velocity Z > 0?
  │   YES: Set Jump Count = 0 (initiated jump)
  │   NO: Set Jump Count = 1 (fell off edge)
  │
  └─ Set Air Control = 0.3

// Generate Movement
Event Generate Movement (Delta Time)
  ├─ Get Input Vector
  ├─ Calculate Air Movement Direction
  ├─ Add Movement Input (Direction, Air Control)
  │
  └─ Branch: Jump Input && Jump Count < Max Air Jumps?
      YES:
      ├─ Launch Character (0, 0, Jump Velocity)
      ├─ Increment Jump Count
      └─ Play Double Jump Effect

// On Exit
Event On Exit
  └─ Reset Jump Count = 0
```

## Using the System in Code

### Assigning Configuration

In your Character Blueprint or C++:

```cpp
// C++
UPROPERTY(EditDefaultsOnly, Category = "Movement")
UFVMovementHandlerDataAsset* MovementConfig;

// Assign in constructor or defaults
UFVCharacterMovementComponent* MovementComp = GetCharacterMovement<UFVCharacterMovementComponent>();
MovementComp->MovementConfig = LoadObject<UFVMovementHandlerDataAsset>(nullptr, TEXT("/Game/Data/DA_DefaultMovementConfig"));
```

Or in Blueprint: Select your character → Movement Component → Movement Config → Choose your data asset

### Working with Tags

#### Adding Tags (e.g., from input)

```cpp
// C++
UFVCharacterMovementComponent* MovementComp = GetCharacterMovement<UFVCharacterMovementComponent>();
MovementComp->AddMovementTag(FFVMovementTags::Get().Movement_Action_Sprint);
```

```blueprint
// Blueprint
Get Character Movement (FV)
└─ Add Movement Tag (Movement.Action.Sprint)
```

#### Removing Tags

```cpp
MovementComp->RemoveMovementTag(FFVMovementTags::Get().Movement_Action_Sprint);
```

#### Checking Tags

```cpp
if (MovementComp->HasMovementTag(FFVMovementTags::Get().Movement_State_Stunned))
{
    // Character is stunned
}
```

### Requesting Movement Modes

```cpp
// Force transition to specific mode
MovementComp->RequestMovementMode(EFVMovementMode::Traversing, true);

// Request mode (will check tags and activation rules)
if (MovementComp->RequestMovementMode(EFVMovementMode::OnGround, false))
{
    // Successfully transitioned
}
```

## Tag Reference

### Movement Modes
- `Movement.Mode.OnGround` - Character on solid ground
- `Movement.Mode.InAir` - Character airborne
- `Movement.Mode.Traversing` - Climbing, mantling, etc.
- `Movement.Mode.Swimming` - In water
- `Movement.Mode.Flying` - Flying/gliding
- `Movement.Mode.Custom` - Custom implementations

### Movement States
- `Movement.State.Crouching` - Character is crouched
- `Movement.State.Sprinting` - Character is sprinting
- `Movement.State.Sliding` - Character is sliding
- `Movement.State.Dodging` - Character is dodging
- `Movement.State.Stunned` - Character cannot move
- `Movement.State.Rooted` - Character locked in place
- `Movement.State.Slowed` - Movement speed reduced
- `Movement.State.Hastened` - Movement speed increased

### Movement Actions
- `Movement.Action.Jump` - Character jumping
- `Movement.Action.Mantle` - Character mantling
- `Movement.Action.Vault` - Character vaulting
- `Movement.Action.Climb` - Character climbing
- `Movement.Action.WallRun` - Character wall running
- `Movement.Action.Roll` - Character rolling

### Movement Restrictions
- `Movement.Restriction.NoJump` - Jumping disabled
- `Movement.Restriction.NoSprint` - Sprinting disabled
- `Movement.Restriction.NoCrouch` - Crouching disabled
- `Movement.Restriction.NoDirectionChange` - Locked direction
- `Movement.Restriction.NoTraversal` - Traversal disabled

### Surface Types
- `Movement.Surface.Walkable` - Normal surface
- `Movement.Surface.Slippery` - Ice, oil, etc.
- `Movement.Surface.Sticky` - Mud, tar, etc.
- `Movement.Surface.Climbable` - Can be climbed
- `Movement.Surface.Water` - Water surface

## Advanced Patterns

### Priority-Based Activation

Higher priority handlers can interrupt lower priority ones:

```
Ground Walking: Priority 50
Sprint: Priority 75 (can interrupt walking)
Dodge: Priority 100 (can interrupt sprint)
Stun: Priority 255 (can interrupt everything)
```

### Minimum Active Time

Prevent interruption for a duration:

```
Dodge Handler:
  - Priority: 100
  - Can Be Interrupted: false
  - Minimum Active Time: 0.8
```

Even higher priority handlers cannot interrupt until 0.8 seconds pass.

### Cooldown System

Prevent rapid re-activation:

```
Dodge Handler:
  - Cooldown Time: 2.0
```

After exiting, handler cannot activate for 2 seconds.

### Maximum Active Time

Force handler exit after duration:

```
Sprint Handler:
  - Maximum Active Time: 10.0
```

After 10 seconds, handler will automatically exit (e.g., stamina depleted).

### Tag-Based Blocking

Create complex activation rules:

```
Sprint Handler:
  Required Tags:
    - Movement.Mode.OnGround (must be grounded)
    - Movement.Action.Sprint (input required)
  
  Blocked By Tags:
    - Movement.State.Stunned (can't sprint while stunned)
    - Movement.State.Crouching (can't sprint while crouched)
    - Movement.Restriction.NoSprint (explicitly disabled)
```

### Custom Data

Pass configuration to handlers:

```
Climb Handler:
  Custom Data:
    ClimbSpeed: "300.0"
    StaminaCostPerSecond: "5.0"
    MaxClimbHeight: "1000.0"

// In Blueprint:
Event On Initialize
  ├─ Get Custom Data ("ClimbSpeed", "300.0") → Climb Speed
  ├─ Convert String to Float
  └─ Store in variable
```

## Migration from Old System

### Old Handler Classes

Your existing C++ handler classes (UFVGroundMovementHandler, etc.) can be converted to Blueprints:

1. Create new Blueprint inheriting from UFVMovementHandlerBase
2. Copy logic from C++ to Blueprint event handlers
3. Update data asset to reference new Blueprint
4. Delete old C++ handler classes

### Old Component Usage

Old code:
```cpp
MovementModeHandlers.Add(EFVMovementMode::OnGround, NewObject<UFVGroundMovementHandler>());
```

New system:
```cpp
// Handlers created automatically from data asset
// Just assign the data asset in editor or constructor
MovementComponent->MovementConfig = MyDataAsset;
```

## Debugging

### Enable Verbose Logging

In Data Asset:
```
Debug Settings:
  Enable Verbose Logging: true
```

Or console command:
```
Log LogFVMovement Verbose
```

### Check Active Handler

```cpp
UFVMovementHandlerBase* Current = MovementComp->GetCurrentMovementHandler();
UE_LOG(LogTemp, Log, TEXT("Current Handler: %s, Active Time: %f"), 
    *Current->GetName(), Current->GetActiveTime());
```

### Validate Configuration

```cpp
FString ErrorMessage;
if (!MovementConfig->ValidateConfiguration(ErrorMessage))
{
    UE_LOG(LogTemp, Error, TEXT("Config errors: %s"), *ErrorMessage);
}
```

### Check Tag State

```cpp
FGameplayTagContainer Tags = MovementComp->GetMovementTags();
UE_LOG(LogTemp, Log, TEXT("Current Tags: %s"), *Tags.ToStringSimple());
```

## Best Practices

1. **Use Tags Liberally** - Tags are cheap, use them to express state clearly
2. **Set Appropriate Priorities** - Higher numbers interrupt lower numbers
3. **Test Minimum Active Time** - Prevents janky interruptions
4. **Configure Cooldowns** - Prevents ability spam
5. **Validate Configurations** - Use built-in validation to catch errors
6. **Document Custom Data** - Add comments explaining keys and formats
7. **Blueprint Over C++** - Handlers should be Blueprint unless performance-critical
8. **Centralize Configuration** - Use data assets, avoid hardcoded values

## Performance Considerations

- Handler transitions are lightweight (~0.1ms)
- Tag checks use native GameplayTag system (very fast)
- Handlers only tick when active
- Blueprint implementations have minimal overhead compared to C++
- Consider C++ for handlers that run every frame with heavy computation

## Network Considerations

- Set `bRequiresAuthority = true` for server-only handlers
- Set `bReplicates = true` for handlers that need to replicate
- Use `bAllowClientPrediction = true` for responsive input
- Tag changes can be replicated through GAS or custom replication

## Troubleshooting

**Handler Not Activating:**
1. Check Required Tags are present
2. Check Blocked By Tags are NOT present
3. Check handler is not on cooldown
4. Check priority vs current handler
5. Enable verbose logging to see activation checks

**Unexpected Transitions:**
1. Check priorities - higher priority handlers will interrupt
2. Check Minimum Active Time on current handler
3. Check Maximum Active Time hasn't been exceeded
4. Review Granted Tags from handlers (may trigger other handlers)

**Tags Not Working:**
1. Ensure tags are defined in FFVMovementTags
2. Check tag spelling exactly matches
3. Verify tags are being added/removed correctly
4. Use debugger to inspect MovementTags container
