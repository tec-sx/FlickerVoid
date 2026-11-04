// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVCharacterModule.h"
#include "Character/FVCharacter.h"
#include "Movement/FVLocomotionComponent.h"
#include "Modules/ModuleManager.h"
#include "RigVMCore/RigVMRegistry.h"

DEFINE_LOG_CATEGORY(LogFVMovement);
DEFINE_LOG_CATEGORY(LogFVAnimation);

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
            { UFVLocomotionComponent::StaticClass(), FRigVMRegistry::ERegisterObjectOperation::Class },
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
