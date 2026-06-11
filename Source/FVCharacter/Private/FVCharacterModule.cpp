// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/FVCharacter.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Modules/ModuleManager.h"
#include "RigVMCore/RigVMRegistry.h"

// Note: Log categories are defined in FVCore module (Logging/FVLogCategories.h)
// Use LogFVCharacter, LogFVMovement, or LogFVAnimation for logging in this module

/**
 * FVCharacter Module Implementation
 * Handles character-related functionality
 */
class FFVCharacterModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        TArray<UScriptStruct*> AllowedStructTypes = {
            FFVCharacterRuntimeState::StaticStruct(),
        };
			
        static TPair<UClass*, FRigVMRegistry::ERegisterObjectOperation> const AllowedObjectTypes[] =
        {
            { AFVCharacter::StaticClass(), FRigVMRegistry::ERegisterObjectOperation::Class },
            { UFVCharacterMovementComponent::StaticClass(), FRigVMRegistry::ERegisterObjectOperation::Class },
        };
	 
        FRigVMRegistry& RigVMRegistry = FRigVMRegistry::Get();
        RigVMRegistry.RegisterStructTypes(AllowedStructTypes);
        RigVMRegistry.RegisterObjectTypes(AllowedObjectTypes);
    }

    virtual void ShutdownModule() override
    {
        // Character module shutdown
    }
};

IMPLEMENT_MODULE(FFVCharacterModule, FlickerVoidCharacter);
