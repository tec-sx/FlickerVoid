#include "FVPlayerController.h"
#include "Input/FVInputComponent.h"
#include "FVCoreTags.h"
#include "Abilities/FVAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FVPlayerCharacter.h"
#include "InputMappingContext.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Player/FVInventoryUIRouterComponent.h"
#include "Player/FVDialogueUIRouterComponent.h"
#include "Player/FVInteractionUIRouterComponent.h"
#include "Player/FVInteractionDebugComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerController)

AFVPlayerController::AFVPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InventoryUIRouterComponent = CreateDefaultSubobject<UFVInventoryUIRouterComponent>(TEXT("InventoryUIRouterComponent"));
	DialogueUIRouterComponent = CreateDefaultSubobject<UFVDialogueUIRouterComponent>(TEXT("DialogueUIRouterComponent"));
	InteractionUIRouterComponent = CreateDefaultSubobject<UFVInteractionUIRouterComponent>(TEXT("InteractionUIRouterComponent"));
	InteractionDebugComponent = CreateDefaultSubobject<UFVInteractionDebugComponent>(TEXT("InteractionDebugComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AFVPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    CachedCharacter = Cast<AFVPlayerCharacter>(InPawn);

    if (CachedCharacter.IsValid())
    {
        InitializeInput();
	}
    else
    {
        FV_LOG_WARNING(LogFVInput, "Possessed pawn is not AFVPlayerCharacter! Input will not be initialized.");
    }

}

void AFVPlayerController::OnUnPossess()
{
    RemoveInputMappingContexts();

    // Remove ability binds
    if (UFVInputComponent* FVIC = Cast<UFVInputComponent>(InputComponent))
    {
        FVIC->RemoveBinds(AbilityBindHandles);
    }

    CachedCharacter.Reset();

    Super::OnUnPossess();
}

UAbilitySystemComponent* AFVPlayerController::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

void AFVPlayerController::InitializeInput()
{
    if (!InputConfig)
    {
        FV_LOG_WARNING(LogFVInput, "PlayerController has no InputConfig assigned!");
        return;
    }

    UFVInputComponent* FVIC = Cast<UFVInputComponent>(InputComponent);
    if (!FVIC)
    {
        FV_LOG_ERROR(LogFVInput, "InputComponent is not UFVInputComponent! Cannot bind inputs.");
        return;
    }

    AddInputMappingContexts();

    FVIC->BindAbilityActions(
        InputConfig,
        this,
        &ThisClass::Input_AbilityInputTagPressed,
        &ThisClass::Input_AbilityInputTagReleased,
        AbilityBindHandles
    );

    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_MoveTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Move, ETriggerEvent::Completed, this, &ThisClass::Input_MoveCompleted, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Crouch, ETriggerEvent::Started, this, &ThisClass::Input_CrouchTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Walk, ETriggerEvent::Started, this, &ThisClass::Input_WalkTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Sprint, ETriggerEvent::Started, this, &ThisClass::Input_SprintTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Sprint, ETriggerEvent::Completed, this, &ThisClass::Input_SprintCompleted, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Jump, ETriggerEvent::Started, this, &ThisClass::Input_JumpStarted, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Jump, ETriggerEvent::Triggered, this, &ThisClass::Input_JumpTriggered, false);
	FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Aim, ETriggerEvent::Started, this, &ThisClass::Input_AimStarted, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Aim, ETriggerEvent::Completed, this, &ThisClass::Input_AimCompleted, false);
}

void AFVPlayerController::AddInputMappingContexts()
{
    auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if (!Subsystem) return;

    for (const TSoftObjectPtr<UInputMappingContext>& IMC : DefaultInputMappings)
    {
        if (UInputMappingContext* LoadedIMC = IMC.LoadSynchronous())
        {
            Subsystem->AddMappingContext(LoadedIMC, DefaultInputPriority);
        }
    }

    bInputMappingsAdded = true;
}

void AFVPlayerController::RemoveInputMappingContexts()
{
    if (!bInputMappingsAdded)
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (!Subsystem)
    {
        return;
    }

    for (const TSoftObjectPtr<UInputMappingContext>& IMC : DefaultInputMappings)
    {
        if (UInputMappingContext* LoadedIMC = IMC.LoadSynchronous())
        {
            Subsystem->RemoveMappingContext(LoadedIMC);
        }
    }

    bInputMappingsAdded = false;
}

// ============================================================================
// NATIVE INPUT CALLBACKS
// ============================================================================

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_MoveTriggered(const FInputActionValue& Value)
{
    AFVPlayerCharacter* FVPlayer = CachedCharacter.Get();
    if (!FVPlayer)
    {
        return;
    }

    FVPlayer->RequestMove(Value.Get<FVector>());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_MoveCompleted(const FInputActionValue& Value)
{
    if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
    {
        FVPlayer->RequestMove(FVector::ZeroVector);
    }
}

void AFVPlayerController::Input_LookTriggered(const FInputActionValue& Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddYawInput(LookVector.X);
    AddPitchInput(LookVector.Y);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_CrouchTriggered(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestCrouch();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_WalkTriggered(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestWalk();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_SprintTriggered(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestSprint(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_SprintCompleted(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestSprint(false);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_JumpStarted(const FInputActionValue& Value)
{
	AFVPlayerCharacter* FVPlayer = CachedCharacter.Get();
	if (!FVPlayer)
	{
		return;
	}

	const UFVCharacterMovementComponent* MovementComponent = FVPlayer->GetFVCharacterMovement();
	if (!MovementComponent || !MovementComponent->IsMovingOnGround())
	{
		return;
	}

	if (FVPlayer->RequestTraverse())
	{
		return;
	}

	FVPlayer->RequestJump();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_JumpTriggered(const FInputActionValue& Value)
{
	AFVPlayerCharacter* FVPlayer = CachedCharacter.Get();
	if (!FVPlayer)
	{
		return;
	}

	const UFVCharacterMovementComponent* MovementComponent = FVPlayer->GetFVCharacterMovement();
	if (!MovementComponent || !MovementComponent->IsFalling())
	{
		return;
	}

	if (FVPlayer->IsTraversing())
	{
		return;
	}

	FVPlayer->RequestTraverse();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_AimStarted(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestAim(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_AimCompleted(const FInputActionValue& Value)
{
	if (AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
	{
		FVPlayer->RequestAim(false);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
    {
        GetFVAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFVPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const AFVPlayerCharacter* FVPlayer = CachedCharacter.Get())
    {
        GetFVAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
    }
}
