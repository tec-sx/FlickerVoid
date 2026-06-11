// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Logging/FVLogSystem.h"
#include "Logging/FVLogCategories.h"

/**
 * FVCore Module Implementation
 * Core module for FlickerVoid project containing shared systems and utilities
 */
class FFVCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // Initialize the global logging system
        FFVLogSystem::Initialize();

        UE_LOG(LogFVCore, Log, TEXT("FlickerVoid Core Module Started"));
    }

    virtual void ShutdownModule() override
    {
        UE_LOG(LogFVCore, Log, TEXT("FlickerVoid Core Module Shutdown"));

        // Shutdown the global logging system
        FFVLogSystem::Shutdown();
    }
};

IMPLEMENT_MODULE(FFVCoreModule, FlickerVoidCore);
