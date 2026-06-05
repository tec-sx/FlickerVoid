// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVWorld, LogFVEnvironment, or LogFVStreaming for logging in this module

/**
 * FVWorld Module Implementation
 * Handles world management, environment, and streaming
 */
class FFVWorldModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // World module startup
    }

    virtual void ShutdownModule() override
    {
        // World module shutdown
    }
};

IMPLEMENT_MODULE(FFVWorldModule, FlickerVoidWorld);