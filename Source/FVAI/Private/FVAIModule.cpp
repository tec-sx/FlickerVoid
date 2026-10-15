// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVAIModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFVAI);

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