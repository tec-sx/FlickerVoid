// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVCharacterMovementComponent.h"
#include "Character/FVCharacter.h"
#include "Movement/FVMovementHandlerBase.h"
#include "Movement/FVMovementHandlerData.h"
#include "Movement/FVMovementHandlerInfo.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

UFVCharacterMovementComponent::UFVCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovementConfig(nullptr)
	, CurrentHandler(nullptr)
	, bIsInitialized(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFVCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = GetFVCharacter();
	if (!Character)
	{
		FV_LOG_ERROR(LogFVMovement, "Owner is not AFVCharacter! Movement handlers will not be initialized.");
		return;
	}

	if (!MovementConfig)
	{
		FV_LOG_ERROR(
			LogFVMovement, 
			"No MovementConfig assigned to UFVCharacterMovementComponent on %s", 
			*GetOwner()->GetName());
		return;
	}

	InitializeHandlers();
}

void UFVCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized)
	{
		return;
	}

	// Re-evaluate every tick so tag/state changes (not just mode changes) can drive handler switches.
	UFVMovementHandlerBase* SelectedHandler = SelectHandler();
	if (SelectedHandler && SelectedHandler != CurrentHandler)
	{
		TransitionToHandler(SelectedHandler);
	}

	if (CurrentHandler && CurrentHandler->IsActive())
	{
		CurrentHandler->TickMovement(DeltaTime);
	}
}

void UFVCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	if (!bIsInitialized)
	{
		return;
	}

	UFVMovementHandlerBase* SelectedHandler = SelectHandler();

	if (!SelectedHandler || SelectedHandler == CurrentHandler)
	{
		return;
	}

	TransitionToHandler(SelectedHandler);
}

//~=============================================================================
// Initialization
//~=============================================================================

void UFVCharacterMovementComponent::InitializeHandlers()
{
	if (!MovementConfig)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot initialize handlers without MovementConfig");
		return;
	}

	FString ValidationError;
	if (!MovementConfig->ValidateConfiguration(ValidationError))
	{
		FV_LOG_WARNING(LogFVMovement, "Movement configuration has warnings:\n%s", *ValidationError);
	}

	RegisteredHandlers.Empty();

	for (const FFVMovementHandlerInfo& Config : MovementConfig->MovementHandlers)
	{
		if (!Config.HandlerClass)
		{
			FV_LOG_WARNING(LogFVMovement, "Skipping handler - no class assigned");
			continue;
		}

		UFVMovementHandlerBase* Handler = NewObject<UFVMovementHandlerBase>(this, Config.HandlerClass);
		if (!Handler)
		{
			FV_LOG_ERROR(LogFVMovement, "Failed to create handler of class %s", *Config.HandlerClass->GetName());
			continue;
		}

		Handler->Initialize(Character, this, Config);

		RegisteredHandlers.Add(Handler);
	}

	bIsInitialized = true;

	// Activate the highest-priority valid handler immediately so the first ticks are not handlerless.
	UFVMovementHandlerBase* InitialHandler = SelectHandler();
	if (InitialHandler)
	{
		TransitionToHandler(InitialHandler);
	}
}

//~=============================================================================
// Handler Management
//~=============================================================================

UFVMovementHandlerBase* UFVCharacterMovementComponent::SelectHandler() const
{
	UFVMovementHandlerBase* BestHandler = nullptr;
	int32 BestPriority = -1;

	for (UFVMovementHandlerBase* Handler : RegisteredHandlers)
	{
		if (!Handler)
		{
			continue;
		}

		const FFVMovementHandlerInfo& Info = Handler->GetConfig();

		if (!Info.CanActivate(ActiveTags))
		{
			continue;
		}

		if (!Handler->Resolve())
		{
			continue;
		}

		if (Info.Priority > BestPriority)
		{
			BestHandler = Handler;
			BestPriority = Info.Priority;
		}
	}

	return BestHandler;
}

void UFVCharacterMovementComponent::TransitionToHandler(UFVMovementHandlerBase* NewHandler)
{
	if (CurrentHandler && CurrentHandler->IsActive())
	{
		const FFVMovementHandlerInfo& CurrentConfig = CurrentHandler->GetConfig();
		for (const FGameplayTag& Tag : CurrentConfig.GrantedTags)
		{
			ActiveTags.RemoveTag(Tag);
		}

		CurrentHandler->Exit();
	}

	CurrentHandler = NewHandler;

	const FFVMovementHandlerInfo& NewConfig = NewHandler->GetConfig();
	for (const FGameplayTag& Tag : NewConfig.GrantedTags)
	{
		ActiveTags.AddTag(Tag);
	}

	CurrentHandler->Enter();
}

TArray<UFVMovementHandlerBase*> UFVCharacterMovementComponent::GetAllHandlers() const
{
	TArray<UFVMovementHandlerBase*> Result;
	for (UFVMovementHandlerBase* Handler : RegisteredHandlers)
	{
		if (Handler)
		{
			Result.Add(Handler);
		}
	}

	return Result;
}

//~=============================================================================
// Runtime Handler Registration
//~=============================================================================

bool UFVCharacterMovementComponent::RegisterHandler(const FFVMovementHandlerInfo& HandlerInfo)
{
	if (!HandlerInfo.HandlerClass)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot register handler without HandlerClass");
		return false;
	}

	UFVMovementHandlerBase* Handler = NewObject<UFVMovementHandlerBase>(this, HandlerInfo.HandlerClass);
	if (!Handler)
	{
		FV_LOG_ERROR(LogFVMovement, "Failed to create handler of class %s", *HandlerInfo.HandlerClass->GetName());
		return false;
	}

	Handler->Initialize(Character, this, HandlerInfo);
	RegisteredHandlers.Add(Handler);

	return true;
}

//~=============================================================================
// Helper Functions
//~=============================================================================

AFVCharacter* UFVCharacterMovementComponent::GetFVCharacter() const
{
	return Character ? Character : Cast<AFVCharacter>(GetOwner());
}

void UFVCharacterMovementComponent::ReloadConfiguration()
{
	if (!MovementConfig)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot reload - no configuration assigned");
		return;
	}

	// Exit current handler
	if (CurrentHandler && CurrentHandler->IsActive())
	{
		CurrentHandler->Exit();
		CurrentHandler = nullptr;
	}

	// Reinitialize — InitializeHandlers selects the initial handler internally.
	bIsInitialized = false;
	InitializeHandlers();

	UE_LOG(LogFVMovement, Log, TEXT("Movement configuration reloaded"));
}

#if WITH_EDITOR
void UFVCharacterMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFVCharacterMovementComponent, MovementConfig))
	{
		if (MovementConfig)
		{
			FString ValidationError;
			if (!MovementConfig->ValidateConfiguration(ValidationError))
			{
				FV_LOG_WARNING(LogFVMovement, "Movement configuration validation:\n%s", *ValidationError);
			}
		}
	}
}
#endif

