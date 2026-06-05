// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVAI, LogFVBehavior, or LogFVPerception for logging in this module

/**
 * FVAI Module Implementation
 * Handles AI and behavior functionality
 */
class FFVAIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // AI module startup
    }

    virtual void ShutdownModule() override
    {
        // AI module shutdown
    }
};

IMPLEMENT_MODULE(FFVAIModule, FlickerVoidAI);