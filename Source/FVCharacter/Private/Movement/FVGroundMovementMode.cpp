#include "Movement/FVGroundMovementMode.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Movement/FVLocomotionInputs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVGroundMovementMode)

UFVGroundMovementMode::UFVGroundMovementMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Reads the ability system and the Mover component, which is only safe on the game thread.
	bSupportsAsync = false;

	Walk.ForwardSpeed = 180.f;
	Walk.StrafeSpeed = 160.f;
	Walk.BackwardSpeed = 150.f;

	Sprint.ForwardSpeed = 700.f;
	Sprint.StrafeSpeed = 700.f;
	Sprint.BackwardSpeed = 700.f;
	Sprint.Acceleration = 300.f;
	Sprint.GroundFriction = 3.f;

	Crouch.ForwardSpeed = 225.f;
	Crouch.StrafeSpeed = 200.f;
	Crouch.BackwardSpeed = 180.f;
}

void UFVGroundMovementMode::GenerateMove_Implementation(const FMoverSimContext& SimContext, const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const
{
	const FFVLocomotionInputs* Inputs = StartState.InputCmd.InputCollection.FindDataByType<FFVLocomotionInputs>();
	const UCharacterMoverComponent* Mover = GetTypedOuter<UCharacterMoverComponent>();

	// The walking logic below reads speeds from the shared settings, so the gait goes there first.
	if (UCommonLegacyMovementSettings* Settings = Inputs && Mover ? Mover->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>() : nullptr)
	{
		const FFVGaitSettings Gait = SelectGait(*Inputs, Mover->IsCrouching());
		Settings->MaxSpeed = GetDirectionalSpeed(Gait, *Inputs) * GetSpeedMultiplier(Mover->GetOwner());
		Settings->Acceleration = Gait.Acceleration;
		Settings->Deceleration = Gait.Deceleration;
		Settings->GroundFriction = Gait.GroundFriction;
		Settings->TurningRate = TurningRate;
	}

	Super::GenerateMove_Implementation(SimContext, StartState, TimeStep, OutProposedMove);
}

FFVGaitSettings UFVGroundMovementMode::SelectGait(const FFVLocomotionInputs& Inputs, bool bCrouching) const
{
	if (bCrouching)
	{
		return Crouch;
	}

	switch (Inputs.Gait)
	{
	case EFVGait::Walking:
		return Walk;
	case EFVGait::Sprinting:
		return Sprint;
	default:
		return Run;
	}
}

float UFVGroundMovementMode::GetDirectionalSpeed(const FFVGaitSettings& Gait, const FFVLocomotionInputs& Inputs)
{
	if (!Inputs.bFacingControl)
	{
		return Gait.ForwardSpeed;
	}

	// 0 = forward, 90 = sideways, 180 = backwards
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(FMath::Abs(Inputs.LocalMoveDirection.Y), Inputs.LocalMoveDirection.X));
	return Angle <= 90.f
		? FMath::Lerp(Gait.ForwardSpeed, Gait.StrafeSpeed, Angle / 90.f)
		: FMath::Lerp(Gait.StrafeSpeed, Gait.BackwardSpeed, (Angle - 90.f) / 90.f);
}

float UFVGroundMovementMode::GetSpeedMultiplier(const AActor* Owner) const
{
	if (!SpeedMultiplierAttribute.IsValid())
	{
		return 1.f;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!ASC || !ASC->HasAttributeSetForAttribute(SpeedMultiplierAttribute))
	{
		return 1.f;
	}

	return ASC->GetNumericAttribute(SpeedMultiplierAttribute);
}
