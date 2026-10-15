// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * FVNarrative Module Implementation
 * Handles narrative systems, dialogue, and quests
 */
class FFVNarrativeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // Narrative module startup
    }

    virtual void ShutdownModule() override
    {
        // Narrative module shutdown
    }
};

IMPLEMENT_MODULE(FFVNarrativeModule, FlickerVoidNarrative);