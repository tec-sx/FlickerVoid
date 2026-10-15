// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

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
