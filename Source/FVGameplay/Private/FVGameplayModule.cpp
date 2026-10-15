// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FVGameplay Module Implementation
 * Handles gameplay systems, abilities, and combat
 */
class FFVGameplayModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // Gameplay module startup
    }

    virtual void ShutdownModule() override
    {
        // Gameplay module shutdown
    }
};

IMPLEMENT_MODULE(FFVGameplayModule, FlickerVoidGameplay);
