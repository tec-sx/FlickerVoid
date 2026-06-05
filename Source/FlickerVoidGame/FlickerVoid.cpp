// Copyright Epic Games, Inc. All Rights Reserved.

#include "FlickerVoid.h"

#include "MoverDataModelTypes.h"
#include "Modules/ModuleManager.h"
#include "Character/FVCharacter.h"
#include "RigVMCore/RigVMRegistry.h"

void FFlickerVoidModule::StartupModule()
{
	FDefaultGameModuleImpl::StartupModule();

	TArray<UScriptStruct*> AllowedStructTypes = { };
			
	static TPair<UClass*, FRigVMRegistry::ERegisterObjectOperation> const AllowedObjectTypes[] =
	{
		{ AFVCharacter::StaticClass(), FRigVMRegistry::ERegisterObjectOperation::Class }
	};
	 
	FRigVMRegistry& RigVMRegistry = FRigVMRegistry::Get();
	RigVMRegistry.RegisterStructTypes(AllowedStructTypes);
	RigVMRegistry.RegisterObjectTypes(AllowedObjectTypes);
}

void FFlickerVoidModule::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, FlickerVoid, "FlickerVoid");

DEFINE_LOG_CATEGORY(LogFlickerVoid)

