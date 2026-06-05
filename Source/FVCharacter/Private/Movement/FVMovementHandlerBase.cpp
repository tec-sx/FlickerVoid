// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVMovementHandlerBase.h"
#include "Character/FVCharacter.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Movement/FVMovementHandlerInfo.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

UFVMovementHandlerBase::UFVMovementHandlerBase()
	: Character(nullptr)
	, MovementComponent(nullptr)
	, bIsActive(false)
	, bIsInitialized(false)
{
}

void UFVMovementHandlerBase::Initialize(AFVCharacter* InCharacter, UFVCharacterMovementComponent* InMovementComponent, const FFVMovementHandlerInfo& InConfig)
{
	if (bIsInitialized)
	{
		FV_LOG_WARNING(LogFVMovement, "Handler %s already initialized", *GetName());
		return;
	}

	if (!InCharacter || !InMovementComponent)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot initialize handler %s with null character or movement component", *GetName());
		return;
	}

	Character = InCharacter;
	MovementComponent = InMovementComponent;
	Config = InConfig;
	bIsInitialized = true;

	OnInitialize(InCharacter, InMovementComponent, InConfig);
}

void UFVMovementHandlerBase::OnInitialize_Implementation(AFVCharacter* InCharacter, UFVCharacterMovementComponent* InMovementComponent, const FFVMovementHandlerInfo& InConfig) { }

void UFVMovementHandlerBase::Enter()
{
	if (!bIsInitialized)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot enter uninitialized handler: %s", *GetName());
		return;
	}

	bIsActive = true;

	OnEnter();
}

void UFVMovementHandlerBase::TickMovement(float DeltaTime)
{
	if (!bIsActive)
	{
		return;
	}

	GenerateMovement(DeltaTime);
}

void UFVMovementHandlerBase::Exit()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	OnExit();
}

void UFVMovementHandlerBase::NotifyActivationBlocked(const FGameplayTagContainer& CurrentTags, const FString& Reason)
{
	OnActivationBlocked(CurrentTags, Reason);
}

bool UFVMovementHandlerBase::GrantsTag(const FGameplayTag& Tag) const
{
	return Config.GrantedTags.HasTag(Tag);
}
