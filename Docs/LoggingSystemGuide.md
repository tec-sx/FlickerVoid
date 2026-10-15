# FlickerVoid Logging Guide

## Log categories

Each module declares its own categories in its module header and defines them in its module `.cpp`. There is no central category list.

- **Plugins:** one category per plugin, named `Log<PluginName>` (for example `LogFVInventorySystem` in `FVInventorySystem.h`). FVFramework modules use `LogFVCore` and `LogFVFacts` (FVCoreRuntime), `LogFVCoreEditor` (FVCoreEditor), `LogFVFlow` (FVCoreFlow), `LogFVCoreUI` (FVCoreUI) and `LogFVDebug` (FVCoreDebug).
- **Game modules:** `LogFlickerVoid` (FlickerVoid), `LogFVMovement` and `LogFVAnimation` (FlickerVoidCharacter, `FVCharacterModule.h`), `LogFVAI` (FlickerVoidAI, `FVAIModule.h`).
- Reuse a plugin's category for code in that domain before adding a new one, and never declare the same name in two modules.

Adding a category to a module:

```cpp
// Public/<Module>.h
<MODULE>_API DECLARE_LOG_CATEGORY_EXTERN(Log<Module>, Log, All);

// Private/<Module>Module.cpp
DEFINE_LOG_CATEGORY(Log<Module>);
```

## FV_LOG macros

`FFVLogSystem` (`FVCore/Public/Logging/FVLogSystem.h`, module FlickerVoidCore) adds macros that prefix the message with `Function:Line`:

```cpp
#include "Logging/FVLogSystem.h"
#include "FVCharacterModule.h"

FV_LOG_ERROR(LogFVMovement, "Invalid movement mode: %d", (int32)Mode);
FV_LOG_WARNING(LogFVAnimation, "Missing anim layer on %s", *GetName());
FV_LOG(LogFVMovement, Verbose, "Speed clamped to %f", Speed);
if (!FV_CHECK(Character != nullptr, LogFVMovement, "Character is null"))
{
    return;
}
```

Plain `UE_LOG` works with any category as usual.

## Verbosity

Console: `Log LogFVMovement Verbose`. Config: a `[Core.Log]` section in `DefaultEngine.ini`, for example `LogFVAI=Warning`.
