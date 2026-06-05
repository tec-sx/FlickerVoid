// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVCharacterMovementComponent.h"
#include "Character/FVCharacter.h"
#include "Character/FVCharacterStateManager.h"
#include "Movement/FVMovementHandlerBase.h"
#include "Movement/FVMovementHandlerData.h"
#include "Movement/FVMovementHandlerInfo.h"
#include "Movement/FVMovementTags.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Character/FVCharacterTags.h"

UFVCharacterMovementComponent::UFVCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovementConfig(nullptr)
	, CurrentMovementHandler(nullptr)
	, LastTransitionTime(0.0f)
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

	if (CurrentMovementHandler && CurrentMovementHandler->IsActive())
	{
		CurrentMovementHandler->TickMovement(DeltaTime);
	}
}

void UFVCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	
	UpdateActiveHandler();
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
}

//~=============================================================================
// Handler Management
//~=============================================================================

void UFVCharacterMovementComponent::UpdateActiveHandler()
{
	if (!bIsInitialized)
	{
		return;
	}

	// Find best handler for current state
	UFVMovementHandlerBase* BestHandler = FindBestHandler();
	
	// Already have the right handler active
	if (BestHandler == CurrentMovementHandler)
	{
		return;
	}

	// Transition to new handler
	TransitionToHandler(BestHandler);
}

void UFVCharacterMovementComponent::TransitionToHandler(UFVMovementHandlerBase* NewHandler)
{
	if (!NewHandler && NewHandler == CurrentMovementHandler)
	{
		return;
	}

	if (!CanTransition())
	{
		FV_LOG(LogFVMovement, Verbose, "Transition blocked by minimum transition time");
		return;
	}

	// Check if new handler can activate
	if (!NewHandler->CheckCanActivate(ActiveTags))
	{
		FV_LOG(LogFVMovement, Verbose, "Handler activation check failed");
		NewHandler->NotifyActivationBlocked(ActiveTags, TEXT("Activation check failed"));
		return;
	}

	// Exit current handler
	if (CurrentMovementHandler && CurrentMovementHandler->IsActive())
	{
		// Remove granted tags from current handler
		const FFVMovementHandlerInfo& CurrentConfig = CurrentMovementHandler->GetConfig();
		for (const FGameplayTag& Tag : CurrentConfig.GrantedTags)
		{
			ActiveTags.RemoveTag(Tag);
		}

		CurrentMovementHandler->Exit();
		UE_LOG(LogFVMovement, Verbose, TEXT("Exited movement handler: %s"), *CurrentMovementHandler->GetName());
	}

	// Enter new handler
	CurrentMovementHandler = NewHandler;

	// Add granted tags from new handler
	const FFVMovementHandlerInfo& NewConfig = NewHandler->GetConfig();
	for (const FGameplayTag& Tag : NewConfig.GrantedTags)
	{
		ActiveTags.AddTag(Tag);
	}

	CurrentMovementHandler->Enter();
	LastTransitionTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogFVMovement, Log, TEXT("Transitioned to handler: %s"), *CurrentMovementHandler->GetName());
}

UFVMovementHandlerBase* UFVCharacterMovementComponent::FindBestHandler() const
{
	if (!MovementConfig)
	{
		return nullptr;
	}

	// Find all handlers that can activate
	TArray<UFVMovementHandlerBase*> ActivatableHandlers;
	for (UFVMovementHandlerBase* Handler : RegisteredHandlers)
	{
		if (Handler && Handler->CheckCanActivate(ActiveTags))
		{
			ActivatableHandlers.Add(Handler);
		}
	}

	// No handlers can activate
	if (ActivatableHandlers.Num() == 0)
	{
		return nullptr;
	}

	// Sort by priority (highest first)
	ActivatableHandlers.Sort([](const UFVMovementHandlerBase& A, const UFVMovementHandlerBase& B)
	{
		return A.GetConfig().Priority > B.GetConfig().Priority;
	});

	// Return highest priority handler
	return ActivatableHandlers[0];
}

bool UFVCharacterMovementComponent::CanTransition() const
{
	// TODO:
	// Here check the conditions and maybe doo the checks in the movement component instead of the handlers.
	// It is semantically more correct to have the movement component manage transition logic.

	if (!MovementConfig || MovementConfig->MinimumTransitionTime <= 0.0f)
	{
		return true;
	}

	float TimeSinceTransition = GetWorld()->GetTimeSeconds() - LastTransitionTime;
	return TimeSinceTransition >= MovementConfig->MinimumTransitionTime;
}

//~=============================================================================
// Public Handler API
//~=============================================================================

bool UFVCharacterMovementComponent::TryActivateHandlerByTags(const FGameplayTagContainer& ActivationTags)
{
	UFVMovementHandlerBase* Handler = GetHandlerByTags(ActivationTags);
	if (!Handler)
	{
		FV_LOG_WARNING(LogFVMovement, "No handler found with activation tags: %s", *ActivationTags.ToStringSimple());
		return false;
	}

	TransitionToHandler(Handler);

	return CurrentMovementHandler == Handler;
}

UFVMovementHandlerBase* UFVCharacterMovementComponent::GetHandlerByTags(const FGameplayTagContainer& ActivationTags) const
{
	for (UFVMovementHandlerBase* Handler : RegisteredHandlers)
	{
		if (Handler)
		{
			const FFVMovementHandlerInfo& Config = Handler->GetConfig();
			// Check if activation tags match exactly
			if (Config.ActivationTags.HasAll(ActivationTags) && ActivationTags.HasAll(Config.ActivationTags))
			{
				return Handler;
			}
		}
	}
	return nullptr;
}

bool UFVCharacterMovementComponent::CanActivateHandlerByTags(const FGameplayTagContainer& ActivationTags) const
{
	UFVMovementHandlerBase* Handler = GetHandlerByTags(ActivationTags);
	if (!Handler)
	{
		return false;
	}

	return Handler->CheckCanActivate(ActiveTags);
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

	// Check if handler with same activation tags already exists
	if (GetHandlerByTags(HandlerInfo.ActivationTags))
	{
		FV_LOG_WARNING(LogFVMovement, "Handler with activation tags %s already registered", 
			*HandlerInfo.ActivationTags.ToStringSimple());
		return false;
	}

	// Create handler instance
	UFVMovementHandlerBase* Handler = NewObject<UFVMovementHandlerBase>(this, HandlerInfo.HandlerClass);
	if (!Handler)
	{
		FV_LOG_ERROR(LogFVMovement, "Failed to create handler of class %s", 
			*HandlerInfo.HandlerClass->GetName());
		return false;
	}

	// Initialize the handler
	Handler->Initialize(Character, this, HandlerInfo);

	// Add to array
	RegisteredHandlers.Add(Handler);

	UE_LOG(LogFVMovement, Log, TEXT("Registered new handler at runtime: %s with tags: %s"), 
		*Handler->GetName(), *HandlerInfo.ActivationTags.ToStringSimple());

	return true;
}

bool UFVCharacterMovementComponent::UnregisterHandlerByTags(const FGameplayTagContainer& ActivationTags)
{
	for (int32 i = 0; i < RegisteredHandlers.Num(); ++i)
	{
		UFVMovementHandlerBase* Handler = RegisteredHandlers[i];
		if (Handler)
		{
			const FFVMovementHandlerInfo& Config = Handler->GetConfig();
			if (Config.ActivationTags.HasAll(ActivationTags) && ActivationTags.HasAll(Config.ActivationTags))
			{
				// Exit handler if it's currently active
				if (Handler == CurrentMovementHandler && Handler->IsActive())
				{
					Handler->Exit();
					CurrentMovementHandler = nullptr;
				}

				RegisteredHandlers.RemoveAt(i);
				UE_LOG(LogFVMovement, Log, TEXT("Unregistered handler with tags: %s"), 
					*ActivationTags.ToStringSimple());
				return true;
			}
		}
	}

	FV_LOG_WARNING(LogFVMovement, "No handler found with activation tags: %s", 
		*ActivationTags.ToStringSimple());
	return false;
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
	if (CurrentMovementHandler && CurrentMovementHandler->IsActive())
	{
		CurrentMovementHandler->Exit();
		CurrentMovementHandler = nullptr;
	}

	// Reinitialize
	bIsInitialized = false;
	InitializeHandlers();
	UpdateActiveHandler();

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

