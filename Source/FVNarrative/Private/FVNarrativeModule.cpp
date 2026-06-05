// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVNarrative, LogFVDialogue, or LogFVQuest for logging in this module

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