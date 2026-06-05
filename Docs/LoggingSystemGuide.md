# FlickerVoid Logging System Guide

## Overview
The FlickerVoid logging system provides a centralized, module-aware logging architecture with a global error handler. All log categories are defined in the Core module and used across all other modules.

## Architecture

### Core Components
1. **FFVLogSystem** (`FVCore/Public/Logging/FVLogSystem.h`)
   - Global error handler
   - Centralized logging utilities
   - Automatic context capture (file, line, function)

2. **FVLogCategories** (`FVCore/Public/Logging/FVLogCategories.h`)
   - All log categories declared in one place
   - Exported for use in all modules
   - Organized by module and subsystem

## Available Log Categories

### Core System
- `LogFVCore` - Core module logging
- `LogFVSystem` - System-level operations

### Character Module
- `LogFVCharacter` - Character system logging
- `LogFVMovement` - Movement-specific logging
- `LogFVAnimation` - Animation system logging

### AI Module
- `LogFVAI` - AI system logging
- `LogFVBehavior` - Behavior tree logging
- `LogFVPerception` - Perception system logging

### Gameplay Module
- `LogFVGameplay` - General gameplay logging
- `LogFVAbility` - Ability system logging
- `LogFVCombat` - Combat system logging

### Items Module
- `LogFVItems` - Item system logging
- `LogFVInventory` - Inventory system logging
- `LogFVEquipment` - Equipment system logging

### Narrative Module
- `LogFVNarrative` - Narrative system logging
- `LogFVDialogue` - Dialogue system logging
- `LogFVQuest` - Quest system logging

### UI Module
- `LogFVUI` - UI system logging
- `LogFVHUD` - HUD logging
- `LogFVMenu` - Menu system logging

### World Module
- `LogFVWorld` - World management logging
- `LogFVEnvironment` - Environment logging
- `LogFVStreaming` - Level streaming logging

### Other Categories
- `LogFVInput` - Input system logging
- `LogFVPhysics` - Physics logging
- `LogFVNetwork` - Network logging
- `LogFVReplication` - Replication logging
- `LogFVSave` - Save/load system logging
- `LogFVAudio` - Audio system logging

## Usage

### Basic Logging Macros

```cpp
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

// Error logging with automatic context (file:line:function)
FV_LOG_ERROR(LogFVCharacter, "Character %s failed to initialize", *CharacterName);

// Warning logging
FV_LOG_WARNING(LogFVMovement, "Invalid movement mode: %d", (int32)Mode);

// Fatal error (terminates application)
FV_LOG_FATAL(LogFVCore, "Critical system failure: %s", *ErrorDetails);

// Conditional check with logging
if (!FV_CHECK(Character != nullptr, LogFVCharacter, "Character is null!"))
{
    return;
}

// General logging with verbosity
FV_LOG(LogFVGameplay, Log, "Game state changed to: %s", *NewStateName);
FV_LOG(LogFVGameplay, Verbose, "Detailed state info: %s", *Details);
```

### Standard UE Logging

You can still use standard Unreal Engine logging macros:

```cpp
#include "Logging/FVLogCategories.h"

UE_LOG(LogFVCharacter, Log, TEXT("Character spawned"));
UE_LOG(LogFVMovement, Warning, TEXT("Movement speed clamped"));
UE_LOG(LogFVAI, Error, TEXT("AI controller initialization failed"));
```

### Global Error Handler Functions

```cpp
#include "Logging/FVLogSystem.h"
#include "Logging/FVLogCategories.h"

// Handle error with context
FFVLogSystem::HandleError(TEXT("Failed to load asset"), LogFVItems, TEXT("ItemManager::LoadItem"));

// Handle warning
FFVLogSystem::HandleWarning(TEXT("Asset not found, using default"), LogFVItems);

// Check condition and log
bool bSuccess = FFVLogSystem::CheckAndLogError(
    Asset != nullptr, 
    TEXT("Failed to load required asset"), 
    LogFVItems
);
```

## Best Practices

### 1. Choose the Appropriate Log Category
```cpp
// GOOD - Specific category
FV_LOG_ERROR(LogFVMovement, "Failed to initialize movement handler");

// BAD - Generic category
FV_LOG_ERROR(LogFVCharacter, "Failed to initialize movement handler");
```

### 2. Include Relevant Context
```cpp
// GOOD - Include object/actor name
FV_LOG_ERROR(LogFVCharacter, "Character %s failed health check", *GetName());

// BAD - Missing context
FV_LOG_ERROR(LogFVCharacter, "Character failed health check");
```

### 3. Use Appropriate Verbosity
```cpp
// Error - Something went wrong and needs attention
FV_LOG_ERROR(LogFVGameplay, "Failed to spawn actor");

// Warning - Something unexpected but handled
FV_LOG_WARNING(LogFVGameplay, "Actor spawn location adjusted due to collision");

// Log - Normal informational message
FV_LOG(LogFVGameplay, Log, "Actor spawned successfully");

// Verbose - Detailed debugging information
FV_LOG(LogFVGameplay, Verbose, "Spawn parameters: Location=%s, Rotation=%s", *Loc.ToString(), *Rot.ToString());
```

### 4. Check Pointers Before Use
```cpp
// GOOD - Check and log in one step
if (!FV_CHECK(Controller != nullptr, LogFVCharacter, "Controller is null"))
{
    return;
}

// GOOD - Manual check with context
if (Controller == nullptr)
{
    FV_LOG_ERROR(LogFVCharacter, "Controller is null for character: %s", *GetName());
    return;
}
```

### 5. Module-Specific Logging
Each module should use its designated log categories:

```cpp
// In FVCharacter module
FV_LOG_ERROR(LogFVCharacter, "Character initialization failed");
FV_LOG_ERROR(LogFVMovement, "Movement component error");

// In FVAI module
FV_LOG_ERROR(LogFVAI, "AI controller error");
FV_LOG_ERROR(LogFVBehavior, "Behavior tree execution failed");

// In FVGameplay module
FV_LOG_ERROR(LogFVGameplay, "Gameplay system error");
FV_LOG_ERROR(LogFVAbility, "Ability activation failed");
```

## Console Commands

### Enable/Disable Log Categories
```
Log LogFVCharacter Verbose      # Enable verbose logging for characters
Log LogFVMovement Warning       # Only show warnings and errors for movement
Log LogFVAI Off                 # Disable AI logging
Log LogFVGameplay All           # Show all gameplay logs
```

### Save Logs
```
Log LogFVCharacter Log          # Set to normal verbosity
```

## Configuration

### DefaultEngine.ini
Add log configuration to your DefaultEngine.ini:

```ini
[Core.Log]
LogFVCore=Log
LogFVCharacter=Log
LogFVMovement=Log
LogFVAI=Log
LogFVGameplay=Log
LogFVItems=Log
LogFVNarrative=Log
LogFVUI=Log
LogFVWorld=Log

; Set to Verbose for detailed debugging
; LogFVMovement=Verbose

; Set to Warning to reduce noise
; LogFVAI=Warning
```

## Adding New Log Categories

To add a new log category:

1. Add declaration in `FVCore/Public/Logging/FVLogCategories.h`:
```cpp
FLICKERVOIDCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogFVNewSystem, Log, All);
```

2. Add definition in `FVCore/Private/Logging/FVLogCategories.cpp`:
```cpp
DEFINE_LOG_CATEGORY(LogFVNewSystem);
```

3. Use in your code:
```cpp
#include "Logging/FVLogCategories.h"
FV_LOG_ERROR(LogFVNewSystem, "New system error");
```

## Module Dependencies

All modules that use the logging system must have `FlickerVoidCore` in their dependencies:

```csharp
// In YourModule.Build.cs
PublicDependencyModuleNames.AddRange(new string[] 
{
    "Core",
    "CoreUObject",
    "Engine",
    "FlickerVoidCore"  // Required for logging
});
```

## Troubleshooting

### Build Errors
If you get linker errors about undefined log categories:
1. Ensure `FlickerVoidCore` is in your module's dependencies
2. Rebuild the solution completely
3. Check that you're including `Logging/FVLogCategories.h`

### Runtime Issues
If logs aren't appearing:
1. Check log verbosity settings in DefaultEngine.ini
2. Use console commands to enable specific categories
3. Verify the logging system initialized (check for "FlickerVoid Logging System Initialized" in logs)

### Performance
The logging system has minimal overhead:
- Macros use compile-time string formatting
- Context capture is only when actually logging
- Can be completely disabled in shipping builds using UE's standard mechanisms
