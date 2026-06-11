#include "Character/FVCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Character/FVCharacterStateManager.h"
#include "Character/FVCharacterArchetypeData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacter)

AFVCharacter::AFVCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UFVCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetReplicatingMovement(false);

	// Create state manager
	StateManager = CreateDefaultSubobject<UFVCharacterStateManager>(TEXT("StateManager"));
}

void AFVCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize state manager
	if (StateManager)
	{
		StateManager->Initialize(this);
	}
}

void AFVCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFVCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (StateManager)
	{
		StateManager->UpdateState(DeltaTime);
	}
}

void AFVCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	LandingVelocity = GetCharacterMovement()->Velocity;
	bJustLanded = true;
	
	
	GetWorld()->GetTimerManager()
		.SetTimer(JustLandedTimerHandle, [this](){ bJustLanded = false; }, 0.3f, false);
}

//~=============================================================================
// Character Properties
//~=============================================================================

float AFVCharacter::GetGroundSpeed() const
{
	float Speed = 0.0f;
	
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		Speed = GetCharacterMovement()->Velocity.Size2D();
	}

	return Speed;
}

FRotator AFVCharacter::GetAimRotation() const
{
	FRotator AimRotation = GetControlRotation();
	if (TargetedActor != nullptr)
	{
		FVector DirectionToTarget = TargetedActor->GetActorLocation() - GetActorLocation();
		
		AimRotation = DirectionToTarget.ToOrientationRotator();
		AimRotation.Roll = AimRotation.Pitch = 0;
	}
	
	return AimRotation;
}

UFVCharacterMovementComponent* AFVCharacter::GetFVCharacterMovement() const
{ 
	return GetCharacterMovement<UFVCharacterMovementComponent>(); 
}

FFVCharacterAnimationData AFVCharacter::GetAnimationData() const
{
	FFVCharacterAnimationData AnimData;

	AnimData.ActorTransform = GetActorTransform();
	AnimData.Velocity = GetCharacterMovement()->Velocity;
	AnimData.GroundSpeed = GetGroundSpeed();
	AnimData.InputAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
	AnimData.CurrentMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	AnimData.CurrentMaxDeceleration = GetCharacterMovement()->GetMaxBrakingDeceleration();
	AnimData.OrientationIntent = GetActorRotation();
	AnimData.AimingDirection = GetAimRotation();
	AnimData.LandingVelocity = LandingVelocity;
	AnimData.GroundNormal = GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal;
	
	return AnimData;
}

FFVCharacterIntent AFVCharacter::GetIntent() const
{
	FFVCharacterIntent Intent;

	Intent.Direction       = MovementDirection;
	Intent.bWantsToWalk    = bIsWalking;
	Intent.bWantsToSprint  = bIsSprinting;
	Intent.bWantsToCrouch  = IsCrouched();
	Intent.bWantsToAim     = bWantsToAim;
	Intent.bWantsToInteract = bWantsToInteract;
	Intent.bWantsToTraverse = bWantsToTraverse;
	Intent.bWantsToJump    = bWantsToJump;

	return Intent;
}

FFVCharacterRuntimeState AFVCharacter::GetRuntimeState() const
{	
	FFVCharacterRuntimeState RuntimeState;
	
	RuntimeState.Velocity = GetCharacterMovement()->Velocity;
	RuntimeState.Acceleration = GetCharacterMovement()->GetCurrentAcceleration();
	// RuntimeState.bIsWalking = IsWalking();

	RuntimeState.bIsFalling = GetCharacterMovement()->IsFalling();
	RuntimeState.bIsCrouching = GetFVCharacterMovement()->IsCrouching();

	return RuntimeState;
}

bool AFVCharacter::CanSprint() const
{
	return true;
}

void AFVCharacter::RequestWalk()
{
	if (IsSprinting())
	{
		return;
	}
	
	IsWalking() ? SetWalking(false) : SetWalking(true);
}

void AFVCharacter::RequestCrouch()
{
	IsCrouched() ? UnCrouch() : Crouch();
}

void AFVCharacter::RequestSprint(const bool bValue)
{
	if (bValue && IsCrouched())
	{
		UnCrouch();
	}
	
	if (CanSprint())
	{
		SetSprinting(bValue);
	}
}

void AFVCharacter::RequestJump()
{
	IsCrouched() ? UnCrouch() : Jump();
}
