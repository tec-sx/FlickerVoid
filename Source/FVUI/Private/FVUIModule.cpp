// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVUI, LogFVHUD, or LogFVMenu for logging in this module

/**
 * FVUI Module Implementation
 * Handles UI, HUD, and menu systems
 */
class FFVUIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // UI module startup
    }

    virtual void ShutdownModule() override
    {
        // UI module shutdown
    }
};

IMPLEMENT_MODULE(FFVUIModule, FlickerVoidUI);
