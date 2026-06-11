#include "FVPlayerController.h"
#include "FVPlayerState.h"
#include "Character/FVCharacter.h"
#include "Input/FVInputConfig.h"
#include "Input/FVInputComponent.h"
#include "FVCoreTags.h"
#include "FVAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/FVCharacterTags.h"
#include "InputMappingContext.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Interaction/FVInteractionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerController)

AFVPlayerController::AFVPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }

void AFVPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // ========================================================================
    // NOTE: Input bindings happen in InitializeInput() after possession
    // SetupInputComponent is called before we have a pawn
    // ========================================================================
}

void AFVPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    CachedCharacter = Cast<AFVCharacter>(InPawn);

    if (CachedCharacter)
    {
        CachedInteractionComponent = CachedCharacter->FindComponentByClass<UFVInteractionComponent>();
        InitializeInput();
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Possessed pawn is not AFVCharacter! Input will not be initialized."));
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

    CachedCharacter = nullptr;
    CachedInteractionComponent = nullptr;

    Super::OnUnPossess();
}

void AFVPlayerController::InitializeInput()
{
    if (!InputConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController has no InputConfig assigned!"));
        return;
    }

    UFVInputComponent* FVIC = Cast<UFVInputComponent>(InputComponent);
    if (!FVIC)
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent is not UFVInputComponent! Cannot bind inputs."));
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
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_CrouchTriggered, false);
    FVIC->BindNativeAction(InputConfig, FVCoreTags::InputTag_Walk, ETriggerEvent::Triggered, this, &ThisClass::Input_WalkTriggered, false);
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

void AFVPlayerController::Input_MoveTriggered(const FInputActionValue& Value)
{
    const FVector MovementVector = Value.Get<FVector>();
    CachedCharacter->RequestMove(MovementVector);
}

void AFVPlayerController::Input_MoveCompleted(const FInputActionValue& Value)
{
    CachedCharacter->RequestMove(FVector::ZeroVector);
}

void AFVPlayerController::Input_LookTriggered(const FInputActionValue& Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddYawInput(LookVector.X);
    AddPitchInput(LookVector.Y);
}

void AFVPlayerController::Input_CrouchTriggered(const FInputActionValue& Value)
{
    CachedCharacter->RequestCrouch();
}

void AFVPlayerController::Input_WalkTriggered(const FInputActionValue& Value)
{
    CachedCharacter->RequestWalk();
}

void AFVPlayerController::Input_SprintTriggered(const FInputActionValue& Value)
{
    CachedCharacter->RequestSprint(true);
}

void AFVPlayerController::Input_SprintCompleted(const FInputActionValue& Value)
{
    CachedCharacter->RequestSprint(false);
}

void AFVPlayerController::Input_JumpStarted(const FInputActionValue& Value)
{
    if (CachedCharacter->GetFVCharacterMovement()->IsMovingOnGround())
    {
        if (CachedCharacter->RequestTraverse())
        {
            return;
        }
        
        CachedCharacter->RequestJump();
    }
}

void AFVPlayerController::Input_JumpTriggered(const FInputActionValue& Value)
{
    if (CachedCharacter->GetFVCharacterMovement()->IsFalling())
    {
        // Continuous check for traversal opportunities while jump button held (e.g., in air)
        if (CachedCharacter->IsTraversing())
        {
            return;
        }
        
        CachedCharacter->RequestTraverse();
    }
}


void AFVPlayerController::Input_AimStarted(const FInputActionValue& Value)
{
	CachedCharacter->RequestAim(true);
}

void AFVPlayerController::Input_AimCompleted(const FInputActionValue& Value)
{
    CachedCharacter->RequestAim(false);
}

void AFVPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (AFVPlayerState* PS = GetPlayerState<AFVPlayerState>())
    {
        if (UFVAbilitySystemComponent* ASC = PS->GetFVAbilitySystemComponent())
        {
            ASC->AbilityInputTagPressed(InputTag);
        }
    }
}

void AFVPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (AFVPlayerState* PS = GetPlayerState<AFVPlayerState>())
    {
        if (UFVAbilitySystemComponent* ASC = PS->GetFVAbilitySystemComponent())
        {
            ASC->AbilityInputTagReleased(InputTag);
        }
    }
}
