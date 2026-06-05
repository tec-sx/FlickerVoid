// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVItems, LogFVInventory, or LogFVEquipment for logging in this module

/**
 * FVItems Module Implementation
 * Handles item, inventory, and equipment systems
 */
class FFVItemsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        // Items module startup
    }

    virtual void ShutdownModule() override
    {
        // Items module shutdown
    }
};

IMPLEMENT_MODULE(FFVItemsModule, FlickerVoidItems);
