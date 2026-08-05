#include "Character/FVCharacter.h"

#include "FlowComponent.h"
#include "FVCharacterTypes.h"
#include "FVCoreTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/FVCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacter)

AFVCharacter::AFVCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UFVCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	SetReplicatingMovement(false);
	
	FlowComponent = CreateDefaultSubobject<UFlowComponent>(TEXT("FlowComponent"));
	
	GetFlowComponent()->IdentityTags = FGameplayTagContainer(FVCoreTags::Player_Pawn);
}


void AFVCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	if (TargetedPosition != FVector::ZeroVector)
	{
		const FVector DirectionToTarget = TargetedPosition - GetActorLocation();
		
		AimRotation = DirectionToTarget.ToOrientationRotator();
		AimRotation.Roll = AimRotation.Pitch = 0;
	}
	
	return AimRotation;
}

UFVCharacterMovementComponent* AFVCharacter::GetFVCharacterMovement() const
{ 
	return GetCharacterMovement<UFVCharacterMovementComponent>(); 
}

FFVCharacterRuntimeState AFVCharacter::GetRuntimeState() const
{	
	FFVCharacterRuntimeState RuntimeState;
	
	RuntimeState.ActorTransform = GetActorTransform();
	RuntimeState.GroundSpeed = GetGroundSpeed();
	RuntimeState.Velocity = GetCharacterMovement()->Velocity;
	RuntimeState.InputAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
	RuntimeState.CurrentMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	RuntimeState.CurrentMaxDeceleration = GetCharacterMovement()->GetMaxBrakingDeceleration();
	RuntimeState.Gait = GetGait();
	RuntimeState.Stance = GetStance();
	RuntimeState.MovementMode = GetMovementMode();
	RuntimeState.OrientationIntent = GetActorRotation();
	RuntimeState.AimingDirection = GetAimRotation();
	RuntimeState.LandingVelocity = LandingVelocity;
	RuntimeState.bJustLanded = bJustLanded;
	RuntimeState.GroundNormal = GetCharacterMovement()->CurrentFloor.HitResult.ImpactNormal;
	
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

FGameplayTagContainer AFVCharacter::GetIdentityTags() const
{
	return GetFlowComponent()->IdentityTags;
}

void AFVCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	TagContainer.AppendTags(OwnedTags);
}

void AFVCharacter::AddGameplayTag(FGameplayTag Tag)
{
	OwnedTags.AddTag(Tag);
}

void AFVCharacter::RemoveGameplayTag(FGameplayTag Tag)
{
	OwnedTags.RemoveTag(Tag);
}

EFVGait AFVCharacter::GetGait() const
{
	if (IsSprinting())
	{
		return EFVGait::Sprinting;
	}
	else if (IsWalking())
	{
		return EFVGait::Walking;
	}
	else
	{
		return EFVGait::Running;
	}
}

EFVStance AFVCharacter::GetStance() const
{
	return IsCrouched() ? EFVStance::Crouch : EFVStance::Stand;
}

EFVMovementMode AFVCharacter::GetMovementMode() const
{
	switch (GetCharacterMovement()->MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return EFVMovementMode::OnGround;
	case MOVE_Falling:
		return EFVMovementMode::InAir;
	case MOVE_Flying:
		return EFVMovementMode::Traversing;
	case MOVE_Swimming:
		return EFVMovementMode::Swimming;
	default:
		return EFVMovementMode::None;
	}
}
