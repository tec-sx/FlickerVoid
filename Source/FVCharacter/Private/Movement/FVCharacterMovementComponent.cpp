// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVCharacterMovementComponent.h"
#include "Character/FVCharacter.h"
#include "Movement/FVMovementHandlerBase.h"
#include "Movement/FVMovementHandlerData.h"
#include "Movement/FVMovementHandlerInfo.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacterMovementComponent)

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
	if (CanInterruptCurrentHandler())
	{
		UFVMovementHandlerBase* SelectedHandler = SelectHandler();
		if (SelectedHandler && SelectedHandler != CurrentHandler)
		{
			TransitionToHandler(SelectedHandler);
		}
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

	if (!CanInterruptCurrentHandler())
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
	const FFVMovementHandlerInfo* BestInfo = nullptr;

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

		if (!BestInfo || Info.HasHigherPriorityThan(*BestInfo))
		{
			BestHandler = Handler;
			BestInfo = &Info;
		}
	}

	return BestHandler;
}

bool UFVCharacterMovementComponent::CanInterruptCurrentHandler() const
{
	if (!CurrentHandler || !CurrentHandler->IsActive())
	{
		return true;
	}

	return CurrentHandler->GetConfig().bCanBeInterrupted;
}

void UFVCharacterMovementComponent::TransitionToHandler(UFVMovementHandlerBase* NewHandler)
{
	if (!NewHandler || NewHandler == CurrentHandler)
	{
		return;
	}

	if (CurrentHandler)
	{
		ActiveTags.RemoveTags(CurrentHandler->GetConfig().GrantedTags);
		CurrentHandler->Exit();
		CurrentHandler = nullptr;
	}

	// Tags are only granted once activation is confirmed, otherwise a failed Enter() leaks them.
	NewHandler->Enter();
	if (!NewHandler->IsActive())
	{
		return;
	}

	CurrentHandler = NewHandler;
	ActiveTags.AppendTags(NewHandler->GetConfig().GrantedTags);
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
	return Character ? Character.Get() : Cast<AFVCharacter>(GetOwner());
}

void UFVCharacterMovementComponent::ReloadConfiguration()
{
	if (!MovementConfig)
	{
		FV_LOG_ERROR(LogFVMovement, "Cannot reload - no configuration assigned");
		return;
	}

	if (CurrentHandler)
	{
		CurrentHandler->Exit();
		CurrentHandler = nullptr;
	}

	ActiveTags.Reset();

	bIsInitialized = false;
	InitializeHandlers();

	FV_LOG_INFO(LogFVMovement, "Movement configuration reloaded");
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

