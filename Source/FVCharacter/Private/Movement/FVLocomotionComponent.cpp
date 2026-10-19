#include "Movement/FVLocomotionComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "FVCharacterModule.h"
#include "FVCharacterTags.h"
#include "GameFramework/Pawn.h"
#include "Movement/FVLocomotionData.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLocomotionComponent)

UFVLocomotionComponent::UFVLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	MovementBlockingTags.AddTag(FVCharacterTags::Status_Movement_Blocked);
}

UFVLocomotionComponent* UFVLocomotionComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVLocomotionComponent>() : nullptr;
}

void UFVLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	Mover = GetOwner()->FindComponentByClass<UCharacterMoverComponent>();
	if (!Mover)
	{
		UE_LOG(LogFVMovement, Error, TEXT("%s has a locomotion component but no Character Mover Component; it won't move."), *GetNameSafe(GetOwner()));
		SetComponentTickEnabled(false);
		return;
	}

	LastOrientationIntent = GetOwner()->GetActorForwardVector();
	Gait = ResolveGait();
	ApplyGaitToMover();
}

void UFVLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const EFVGait NewGait = ResolveGait();
	if (NewGait != Gait)
	{
		Gait = NewGait;
		OnGaitChanged.Broadcast(Gait);
	}

	ApplyGaitToMover();
	UpdateMovementEvents(DeltaTime);
}

void UFVLocomotionComponent::ProduceInput(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult)
{
	FCharacterDefaultInputs& Inputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<FCharacterDefaultInputs>();

	const APawn* Pawn = Cast<APawn>(GetOwner());
	Inputs.ControlRotation = Pawn && Pawn->GetController() ? Pawn->GetControlRotation() : GetOwner()->GetActorRotation();

	const FRotator ControlYaw = GetControlYaw();
	FVector MoveIntent = FVector::ZeroVector;
	if (CanMove())
	{
		const FVector LocalInput(MoveDirection.X, MoveDirection.Y, 0.f);
		MoveIntent = ControlYaw.RotateVector(LocalInput).GetClampedToMaxSize(1.f) * GetDirectionalSpeedScale();
	}

	Inputs.SetMoveInput(EMoveInputType::DirectionalIntent, MoveIntent);

	if (ShouldFaceControl())
	{
		LastOrientationIntent = ControlYaw.Vector();
	}
	else if (!MoveIntent.IsNearlyZero())
	{
		LastOrientationIntent = MoveIntent.GetSafeNormal();
	}
	Inputs.OrientationIntent = LastOrientationIntent;

	Inputs.bIsJumpJustPressed = bJumpJustPressed;
	Inputs.bIsJumpPressed = bJumpHeld;
	bJumpJustPressed = false;
}

void UFVLocomotionComponent::RequestMove(FVector Direction)
{
	MoveDirection = Direction;
}

void UFVLocomotionComponent::ToggleWalk()
{
	if (!bWantsToSprint)
	{
		bWantsToWalk = !bWantsToWalk;
	}
}

void UFVLocomotionComponent::SetSprinting(bool bValue)
{
	if (bValue)
	{
		StopCrouching();
	}

	bWantsToSprint = bValue;
}

void UFVLocomotionComponent::ToggleCrouch()
{
	if (!Mover)
	{
		return;
	}

	if (Mover->IsCrouching())
	{
		Mover->UnCrouch();
	}
	else if (Mover->CanCrouch())
	{
		Mover->Crouch();
	}
}

void UFVLocomotionComponent::StopCrouching()
{
	if (Mover && Mover->IsCrouching())
	{
		Mover->UnCrouch();
	}
}

void UFVLocomotionComponent::StartJump()
{
	if (IsCrouching())
	{
		StopCrouching();
		return;
	}

	bJumpJustPressed = true;
	bJumpHeld = true;
}

void UFVLocomotionComponent::StopJump()
{
	bJumpHeld = false;
}

void UFVLocomotionComponent::SetAiming(bool bValue)
{
	bAiming = bValue;
}

void UFVLocomotionComponent::SetTraversing(bool bValue)
{
	bTraversing = bValue;
}

EFVStance UFVLocomotionComponent::GetStance() const
{
	return IsCrouching() ? EFVStance::Crouch : EFVStance::Stand;
}

EFVMovementMode UFVLocomotionComponent::GetMovementMode() const
{
	if (!Mover)
	{
		return EFVMovementMode::None;
	}

	if (bTraversing)
	{
		return EFVMovementMode::Traversing;
	}

	if (Mover->IsSlopeSliding())
	{
		return EFVMovementMode::Sliding;
	}

	if (Mover->IsOnGround())
	{
		return EFVMovementMode::OnGround;
	}

	if (Mover->IsSwimming())
	{
		return EFVMovementMode::Swimming;
	}

	return Mover->IsAirborne() ? EFVMovementMode::InAir : EFVMovementMode::None;
}

bool UFVLocomotionComponent::IsCrouching() const
{
	return Mover && Mover->IsCrouching();
}

bool UFVLocomotionComponent::IsOnGround() const
{
	return Mover && Mover->IsOnGround();
}

bool UFVLocomotionComponent::IsFalling() const
{
	return Mover && Mover->IsFalling();
}

bool UFVLocomotionComponent::CanMove() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Mover || (Pawn && Pawn->IsMoveInputIgnored()))
	{
		return false;
	}

	if (MovementBlockingTags.IsEmpty())
	{
		return true;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	return !ASC || !ASC->HasAnyMatchingGameplayTags(MovementBlockingTags);
}

float UFVLocomotionComponent::GetGroundSpeed() const
{
	return IsOnGround() ? Mover->GetVelocity().Size2D() : 0.f;
}

FFVCharacterRuntimeState UFVLocomotionComponent::GetRuntimeState() const
{
	FFVCharacterRuntimeState State;

	const AActor* Owner = GetOwner();
	State.ActorTransform = Owner->GetActorTransform();
	State.Gait = Gait;
	State.Stance = GetStance();
	State.MovementMode = GetMovementMode();
	State.OrientationIntent = LastOrientationIntent.Rotation();
	State.AimingDirection = GetControlYaw();
	State.LandingVelocity = LandingVelocity;
	State.bJustLanded = JustLanded();

	if (!Mover)
	{
		return State;
	}

	const FFVGaitSettings& GaitSettings = GetGaitSettings();
	State.Velocity = Mover->GetVelocity();
	State.GroundSpeed = GetGroundSpeed();
	State.InputAcceleration = Mover->GetMovementIntent() * GaitSettings.Acceleration;
	State.CurrentMaxAcceleration = GaitSettings.Acceleration;
	State.CurrentMaxDeceleration = GaitSettings.Deceleration;

	FHitResult FloorHit;
	if (Mover->TryGetFloorCheckHitResult(FloorHit))
	{
		State.GroundNormal = FloorHit.ImpactNormal;
	}

	return State;
}

const UFVLocomotionData& UFVLocomotionComponent::GetData() const
{
	return LocomotionData ? *LocomotionData : *GetDefault<UFVLocomotionData>();
}

const FFVGaitSettings& UFVLocomotionComponent::GetGaitSettings() const
{
	const UFVLocomotionData& Data = GetData();
	if (IsCrouching())
	{
		return Data.Crouch;
	}

	switch (Gait)
	{
	case EFVGait::Walking:
		return Data.Walk;
	case EFVGait::Sprinting:
		return Data.Sprint;
	default:
		return Data.Run;
	}
}

EFVGait UFVLocomotionComponent::ResolveGait() const
{
	if (bWantsToSprint && !IsCrouching() && !MoveDirection.IsNearlyZero())
	{
		const FVector FacingDirection = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
		const FVector InputDirection = GetControlYaw().RotateVector(FVector(MoveDirection.X, MoveDirection.Y, 0.f)).GetSafeNormal2D();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(FacingDirection, InputDirection), -1.f, 1.f)));

		if (Angle <= GetData().SprintAngleThreshold)
		{
			return EFVGait::Sprinting;
		}
	}

	return bWantsToWalk ? EFVGait::Walking : EFVGait::Running;
}

bool UFVLocomotionComponent::ShouldFaceControl() const
{
	return bAiming || !FMath::IsNearlyZero(MoveDirection.Y);
}

float UFVLocomotionComponent::GetDirectionalSpeedScale() const
{
	const FFVGaitSettings& GaitSettings = GetGaitSettings();
	if (!ShouldFaceControl() || GaitSettings.ForwardSpeed <= UE_KINDA_SMALL_NUMBER)
	{
		return 1.f;
	}

	// 0 = forward, 90 = sideways, 180 = backwards
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(MoveDirection.Y), MoveDirection.X));
	const float Speed = Angle <= 90.f
		? FMath::Lerp(GaitSettings.ForwardSpeed, GaitSettings.StrafeSpeed, Angle / 90.f)
		: FMath::Lerp(GaitSettings.StrafeSpeed, GaitSettings.BackwardSpeed, (Angle - 90.f) / 90.f);

	return Speed / GaitSettings.ForwardSpeed;
}

float UFVLocomotionComponent::GetSpeedMultiplier() const
{
	const FGameplayAttribute& Attribute = GetData().SpeedMultiplierAttribute;
	if (!Attribute.IsValid())
	{
		return 1.f;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC || !ASC->HasAttributeSetForAttribute(Attribute))
	{
		return 1.f;
	}

	return ASC->GetNumericAttribute(Attribute);
}

FRotator UFVLocomotionComponent::GetControlYaw() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const FRotator Rotation = Pawn && Pawn->GetController() ? Pawn->GetControlRotation() : GetOwner()->GetActorRotation();
	return FRotator(0.f, Rotation.Yaw, 0.f);
}

void UFVLocomotionComponent::ApplyGaitToMover() const
{
	UCommonLegacyMovementSettings* Settings = Mover ? Mover->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>() : nullptr;
	if (!Settings)
	{
		return;
	}

	const FFVGaitSettings& GaitSettings = GetGaitSettings();
	Settings->MaxSpeed = GaitSettings.ForwardSpeed * GetSpeedMultiplier();
	Settings->Acceleration = GaitSettings.Acceleration;
	Settings->Deceleration = GaitSettings.Deceleration;
	Settings->GroundFriction = GaitSettings.GroundFriction;
	Settings->TurningRate = GetData().TurningRate;
}

void UFVLocomotionComponent::UpdateMovementEvents(float DeltaTime)
{
	JustLandedTimeLeft = FMath::Max(0.f, JustLandedTimeLeft - DeltaTime);

	const bool bOnGround = Mover->IsOnGround();
	const bool bFalling = Mover->IsFalling();

	if (bWasOnGround && bFalling)
	{
		OnStartedFalling.Broadcast();
	}

	if (bWasFalling && bOnGround)
	{
		LandingVelocity = LastVelocity;
		JustLandedTimeLeft = JustLandedDuration;
		OnLanded.Broadcast(LandingVelocity);
	}

	bWasOnGround = bOnGround;
	bWasFalling = bFalling;
	LastVelocity = Mover->GetVelocity();
}
