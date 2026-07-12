// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/FVAICharacterStateTreeSchema.h"

#include "FVAICharacterController.h"
#include "Actors/FVAICharacter.h"

TArray<FStateTreeExternalDataDesc> UFVAICharacterStateTreeSchema::ContextDataDescriptors;

UFVAICharacterStateTreeSchema::UFVAICharacterStateTreeSchema()
{
	AIControllerClass = AFVAICharacterController::StaticClass();
	ContextActorClass = AFVAICharacter::StaticClass();
	
	if (ContextDataDescriptors.IsEmpty())
	{
		ContextDataDescriptors.Add(FStateTreeExternalDataDesc(
			TEXT("Actor"),
			AFVAICharacter::StaticClass(),
			FGuid::NewGuid()));
		
		ContextDataDescriptors.Add(FStateTreeExternalDataDesc(
			TEXT("AIController"),
			AFVAICharacterController::StaticClass(),
			FGuid::NewGuid()));
	}
}

TConstArrayView<FStateTreeExternalDataDesc> UFVAICharacterStateTreeSchema::GetContextDataDescs() const
{
	return ContextDataDescriptors;
}

bool UFVAICharacterStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	return true; // Allow all structs for now
}
