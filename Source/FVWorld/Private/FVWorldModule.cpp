// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

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