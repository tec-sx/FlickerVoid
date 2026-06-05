// Copyright FlickerVoid. All Rights Reserved.

#include "RigUnits/FVRigUnit_ArmSwingPhase.h"
#include "ControlRig.h"
#include "Units/RigUnitContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRigUnit_ArmSwingPhase)

// ============================================================================
// Constants
// ============================================================================

namespace FVArmSwingConstants
{
	// Base swing amplitudes (scaled by SwingAmount)
	constexpr float BaseForwardSwing = 20.0f;    // cm forward/back
	constexpr float BaseLateralSwing = 4.0f;     // cm outward at extremes
	constexpr float BaseVerticalSwing = 6.0f;    // cm upward at extremes
	
	// Wrist rotation at swing extremes (degrees)
	constexpr float BaseWristPitch = 15.0f;      // Wrist tilts forward/back
	constexpr float BaseWristRoll = 8.0f;        // Wrist rolls slightly
	
	// Elbow behavior
	constexpr float BaseElbowBend = 12.0f;       // Extra elbow out during forward swing
	constexpr float BaseElbowDrop = 6.0f;        // Elbow drops during swing
	
	// Spring-damper physics ranges based on stiffness
	// Stiffness 0 (relaxed): Low spring, underdamped (bouncy)
	// Stiffness 1 (tense): High spring, overdamped (rigid)
	constexpr float MinSpringK = 8.0f;           // Relaxed spring constant
	constexpr float MaxSpringK = 80.0f;          // Tense spring constant
	constexpr float MinDamping = 0.3f;           // Relaxed = underdamped, bouncy
	constexpr float MaxDamping = 1.2f;           // Tense = overdamped, rigid
}

// ============================================================================
// Helper Functions
// ============================================================================

namespace FVArmSwingUtils
{
	/**
	 * Generate deterministic noise based on time and seed.
	 * Returns value in range [-1, 1].
	 */
	float GenerateNoise(float Time, int32 Seed, float Frequency = 1.0f)
	{
		// Multi-octave sine-based noise for smooth variation
		float Noise = 0.0f;
		float SeedOffset = static_cast<float>(Seed) * 0.7123f;
		
		Noise += FMath::Sin((Time * Frequency + SeedOffset) * 2.13f) * 0.5f;
		Noise += FMath::Sin((Time * Frequency * 1.7f + SeedOffset) * 3.51f) * 0.3f;
		Noise += FMath::Sin((Time * Frequency * 2.3f + SeedOffset) * 5.17f) * 0.2f;
		
		return FMath::Clamp(Noise, -1.0f, 1.0f);
	}

	/**
	 * Calculate base swing factor from phase.
	 * Uses sine wave for natural pendulum motion.
	 */
	float CalculateBaseSwingFactor(float Phase, bool bIsLeftArm)
	{
		// Invert phase for left arm (contralateral coordination)
		float EffectivePhase = bIsLeftArm ? -Phase : Phase;
		
		// Sine wave: Phase [-1,1] -> SwingFactor [-1,1]
		return FMath::Sin(EffectivePhase * PI * 0.5f);
	}

	/**
	 * Apply randomness to a value.
	 */
	float ApplyRandomness(float Value, float Randomness, float NoiseValue)
	{
		return Value * (1.0f + NoiseValue * Randomness);
	}

	/**
	 * Calculate spring-damper parameters from stiffness.
	 * 
	 * Stiffness 0 (relaxed):
	 *   - Low spring constant = slow return to rest
	 *   - Underdamped = overshoot and oscillation
	 *   - Feels "floppy"
	 * 
	 * Stiffness 1 (tense):
	 *   - High spring constant = quick return
	 *   - Overdamped = no overshoot
	 *   - Feels "rigid"
	 */
	void GetSpringDamperParams(float Stiffness, float& OutSpringK, float& OutDamping)
	{
		using namespace FVArmSwingConstants;
		
		// Exponential interpolation for more natural feel
		float T = Stiffness * Stiffness; // Quadratic for more response at low stiffness
		
		OutSpringK = FMath::Lerp(MinSpringK, MaxSpringK, T);
		OutDamping = FMath::Lerp(MinDamping, MaxDamping, Stiffness);
	}

	/**
	 * Simulate spring-damper physics.
	 * Updates position offset and velocity based on target.
	 */
	void SimulateSpringDamper(
		FVector& CurrentOffset,
		FVector& Velocity,
		const FVector& TargetOffset,
		float SpringK,
		float Damping,
		float DeltaTime)
	{
		// Spring force: F = -k * (current - target)
		FVector Displacement = CurrentOffset - TargetOffset;
		FVector SpringForce = -SpringK * Displacement;
		
		// Damping force: F = -c * velocity
		FVector DampingForce = -Damping * Velocity * SpringK; // Scale damping with spring
		
		// Total acceleration (mass = 1 for simplicity)
		FVector Acceleration = SpringForce + DampingForce;
		
		// Semi-implicit Euler integration
		Velocity += Acceleration * DeltaTime;
		CurrentOffset += Velocity * DeltaTime;
		
		// Clamp velocity to prevent instability
		const float MaxVelocity = 500.0f;
		Velocity = Velocity.GetClampedToMaxSize(MaxVelocity);
	}

	/**
	 * Calculate wrist swing target offset.
	 */
	FVector CalculateWristTargetOffset(
		float SwingFactor,
		float SwingAmount,
		float Stiffness,
		const FVector& ForwardAxis,
		const FVector& RightAxis,
		const FVector& UpAxis,
		bool bIsLeftArm)
	{
		using namespace FVArmSwingConstants;
		
		// Scale amplitudes by swing amount and stiffness
		// Lower stiffness = slightly larger swing (relaxed people swing more)
		float StiffnessSwingMod = FMath::Lerp(1.15f, 0.9f, Stiffness);
		float EffectiveSwing = SwingAmount * StiffnessSwingMod;
		
		// Longitudinal (forward/back) - primary swing
		FVector ForwardOffset = ForwardAxis * BaseForwardSwing * SwingFactor * EffectiveSwing;
		
		// Lateral (outward) - arms swing slightly outward
		// Maximum at swing extremes, uses absolute value
		float LateralSign = bIsLeftArm ? -1.0f : 1.0f;
		float LateralFactor = FMath::Abs(SwingFactor);
		FVector LateralOffset = RightAxis * BaseLateralSwing * LateralFactor * LateralSign * EffectiveSwing;
		
		// Vertical (up at extremes) - arm rises at swing peaks
		float VerticalFactor = FMath::Abs(SwingFactor);
		FVector VerticalOffset = UpAxis * BaseVerticalSwing * VerticalFactor * EffectiveSwing;
		
		return ForwardOffset + LateralOffset + VerticalOffset;
	}

	/**
	 * Calculate wrist rotation offset.
	 */
	FRotator CalculateWristRotationOffset(
		float SwingFactor,
		float SwingAmount,
		float Stiffness,
		bool bIsLeftArm)
	{
		using namespace FVArmSwingConstants;
		
		FRotator RotOffset = FRotator::ZeroRotator;
		
		// Wrist pitches forward during forward swing, back during back swing
		RotOffset.Pitch = BaseWristPitch * SwingFactor * SwingAmount;
		
		// Wrist rolls slightly - increases with relaxation (low stiffness)
		float RollAmount = FMath::Lerp(1.5f, 0.3f, Stiffness);
		float RollSign = bIsLeftArm ? -1.0f : 1.0f;
		RotOffset.Roll = BaseWristRoll * SwingFactor * SwingAmount * RollAmount * RollSign;
		
		return RotOffset;
	}

	/**
	 * Calculate elbow pole vector offset.
	 */
	FVector CalculateElbowTargetOffset(
		float SwingFactor,
		float SwingAmount,
		float Stiffness,
		const FVector& RightAxis,
		const FVector& UpAxis,
		bool bIsLeftArm)
	{
		using namespace FVArmSwingConstants;
		
		// During forward swing, elbow bends more (points outward)
		// This effect is reduced with higher stiffness (tense = straight arm)
		float BendMultiplier = FMath::Lerp(1.5f, 0.4f, Stiffness);
		float ForwardSwing = FMath::Max(0.0f, SwingFactor);
		
		float LateralSign = bIsLeftArm ? -1.0f : 1.0f;
		FVector LateralOffset = RightAxis * BaseElbowBend * ForwardSwing * BendMultiplier * LateralSign * SwingAmount;
		
		// Elbow also drops during swing (more when relaxed)
		FVector VerticalOffset = UpAxis * -BaseElbowDrop * FMath::Abs(SwingFactor) * BendMultiplier * SwingAmount;
		
		return LateralOffset + VerticalOffset;
	}
}

// ============================================================================
// FFVRigUnit_ArmSwingPhase
// ============================================================================

FRigVMStructUpgradeInfo FFVRigUnit_ArmSwingPhase::GetUpgradeInfo() const
{
	return FRigVMStructUpgradeInfo();
}

FFVRigUnit_ArmSwingPhase_Execute()
{
	using namespace FVArmSwingUtils;
	
	// Early out
	if (BlendWeight <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	float DeltaTime = ExecuteContext.GetDeltaTime();
	
	// Validate required inputs
	if (!WristReference.IsValid() || !ShoulderReference.IsValid())
	{
		return;
	}

	// Get bone indices
	const int32 WristIndex = Hierarchy->GetIndex(WristReference);
	const int32 ShoulderIndex = Hierarchy->GetIndex(ShoulderReference);
	const int32 ElbowPoleIndex = ElbowPoleReference.IsValid() ? Hierarchy->GetIndex(ElbowPoleReference) : INDEX_NONE;

	if (WristIndex == INDEX_NONE || ShoulderIndex == INDEX_NONE)
	{
		return;
	}

	// Get reference transforms
	FTransform WristRefTransform = Hierarchy->GetGlobalTransform(WristIndex);
	FTransform ShoulderRefTransform = Hierarchy->GetGlobalTransform(ShoulderIndex);

	// Initialize on first frame
	if (!bInitialized)
	{
		WristVelocity = FVector::ZeroVector;
		WristSimOffset = FVector::ZeroVector;
		ElbowVelocity = FVector::ZeroVector;
		ElbowSimOffset = FVector::ZeroVector;
		WristRotVelocity = FRotator::ZeroRotator;
		WristSimRotation = FRotator::ZeroRotator;
		AccumulatedTime = 0.0f;
		bInitialized = true;
	}

	// Update time for noise
	AccumulatedTime += DeltaTime;

	// Generate noise values for randomness
	float NoiseAmplitude = GenerateNoise(AccumulatedTime, RandomSeed, 0.8f);
	float NoiseTiming = GenerateNoise(AccumulatedTime, RandomSeed + 1000, 1.2f);
	float NoiseStiffness = GenerateNoise(AccumulatedTime, RandomSeed + 2000, 0.5f);

	// Apply randomness to phase (subtle timing variation)
	float EffectivePhase = Phase + NoiseTiming * Randomness * 0.1f;
	EffectivePhase = FMath::Clamp(EffectivePhase, -1.0f, 1.0f);

	// Calculate base swing factor
	SwingFactor = CalculateBaseSwingFactor(EffectivePhase, bIsLeftArm);
	
	// Apply randomness to amplitude
	float EffectiveSwingAmount = ApplyRandomness(SwingAmount, Randomness, NoiseAmplitude);
	
	// Slight stiffness variation for organic feel
	float EffectiveStiffness = FMath::Clamp(Stiffness + NoiseStiffness * Randomness * 0.15f, 0.0f, 1.0f);

	// Get coordinate frame from shoulder
	FVector ForwardAxis = ShoulderRefTransform.GetRotation().GetForwardVector();
	FVector RightAxis = ShoulderRefTransform.GetRotation().GetRightVector();
	FVector UpAxis = ShoulderRefTransform.GetRotation().GetUpVector();

	// Calculate target offsets (where swing wants to go)
	FVector WristTargetOffset = CalculateWristTargetOffset(
		SwingFactor, EffectiveSwingAmount, EffectiveStiffness,
		ForwardAxis, RightAxis, UpAxis, bIsLeftArm);

	FRotator WristTargetRotOffset = CalculateWristRotationOffset(
		SwingFactor, EffectiveSwingAmount, EffectiveStiffness, bIsLeftArm);

	FVector ElbowTargetOffset = CalculateElbowTargetOffset(
		SwingFactor, EffectiveSwingAmount, EffectiveStiffness,
		RightAxis, UpAxis, bIsLeftArm);

	// Get spring-damper parameters from stiffness
	float SpringK, Damping;
	GetSpringDamperParams(EffectiveStiffness, SpringK, Damping);

	// Simulate spring-damper physics for smooth, organic motion
	if (DeltaTime > 0.0f)
	{
		SimulateSpringDamper(WristSimOffset, WristVelocity, WristTargetOffset, SpringK, Damping, DeltaTime);
		SimulateSpringDamper(ElbowSimOffset, ElbowVelocity, ElbowTargetOffset, SpringK, Damping, DeltaTime);
		
		// Simplified rotation spring (just lerp based on stiffness)
		float RotLerpSpeed = FMath::Lerp(5.0f, 30.0f, EffectiveStiffness);
		WristSimRotation = FMath::RInterpTo(WristSimRotation, WristTargetRotOffset, DeltaTime, RotLerpSpeed);
	}

	// Apply blend weight
	FVector FinalWristOffset = WristSimOffset * BlendWeight;
	FRotator FinalWristRotOffset = WristSimRotation * BlendWeight;
	FVector FinalElbowOffset = ElbowSimOffset * BlendWeight;

	// Calculate final outputs
	WristTargetPosition = WristRefTransform.GetLocation() + FinalWristOffset;
	
	// Combine reference rotation with swing rotation offset
	FQuat RefRotation = WristRefTransform.GetRotation();
	FQuat RotOffset = FinalWristRotOffset.Quaternion();
	WristTargetRotation = RefRotation * RotOffset;

	// Elbow pole target
	if (ElbowPoleIndex != INDEX_NONE)
	{
		FTransform ElbowPoleRefTransform = Hierarchy->GetGlobalTransform(ElbowPoleIndex);
		ElbowPoleTarget = ElbowPoleRefTransform.GetLocation() + FinalElbowOffset;
	}
	else
	{
		// Default pole position
		float LateralSign = bIsLeftArm ? -1.0f : 1.0f;
		ElbowPoleTarget = ShoulderRefTransform.GetLocation() + RightAxis * 30.0f * LateralSign + FinalElbowOffset;
	}

	// Debug visualization
	if (bDrawDebug && ExecuteContext.GetDrawInterface() != nullptr)
	{
		FRigVMDrawInterface* DrawInterface = ExecuteContext.GetDrawInterface();
		const FTransform& WorldToGlobal = ExecuteContext.GetToWorldSpaceTransform().Inverse();
		
		// Reference position (blue)
		DrawInterface->DrawPoint(WorldToGlobal,WristRefTransform.GetLocation(), 6.0f, FLinearColor::Blue);
		
		// Target before spring (yellow) - where swing WANTS to go
		FVector TargetBeforeSpring = WristRefTransform.GetLocation() + WristTargetOffset * BlendWeight;
		DrawInterface->DrawPoint(WorldToGlobal, TargetBeforeSpring, 4.0f, FLinearColor::Yellow);
		
		// Final position after spring (green) - where it actually is
		DrawInterface->DrawPoint(WorldToGlobal, WristTargetPosition, 10.0f, FLinearColor::Green);
		
		// Line showing spring "lag"
		DrawInterface->DrawLine(WorldToGlobal, TargetBeforeSpring, WristTargetPosition, FLinearColor(1.0f, 0.5f, 0.0f), 1.0f);
		
		// Elbow pole
		DrawInterface->DrawPoint(WorldToGlobal, ElbowPoleTarget, 6.0f, FLinearColor::Red);
		
		// Shoulder
		DrawInterface->DrawPoint(WorldToGlobal, ShoulderRefTransform.GetLocation(), 5.0f, FLinearColor::White);
	}
}

// ============================================================================
// FFVRigUnit_ArmSwingPhasePair
// ============================================================================

FRigVMStructUpgradeInfo FFVRigUnit_ArmSwingPhasePair::GetUpgradeInfo() const
{
	return FRigVMStructUpgradeInfo();
}

FFVRigUnit_ArmSwingPhasePair_Execute()
{
	using namespace FVArmSwingUtils;
	
	if (BlendWeight <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	float DeltaTime = ExecuteContext.GetDeltaTime();

	// Get indices
	const int32 LeftWristIdx = LeftWristReference.IsValid() ? Hierarchy->GetIndex(LeftWristReference) : INDEX_NONE;
	const int32 LeftShoulderIdx = LeftShoulderReference.IsValid() ? Hierarchy->GetIndex(LeftShoulderReference) : INDEX_NONE;
	const int32 LeftElbowIdx = LeftElbowPoleReference.IsValid() ? Hierarchy->GetIndex(LeftElbowPoleReference) : INDEX_NONE;

	const int32 RightWristIdx = RightWristReference.IsValid() ? Hierarchy->GetIndex(RightWristReference) : INDEX_NONE;
	const int32 RightShoulderIdx = RightShoulderReference.IsValid() ? Hierarchy->GetIndex(RightShoulderReference) : INDEX_NONE;
	const int32 RightElbowIdx = RightElbowPoleReference.IsValid() ? Hierarchy->GetIndex(RightElbowPoleReference) : INDEX_NONE;

	// Initialize
	if (!bInitialized)
	{
		LeftWristVelocity = FVector::ZeroVector;
		LeftWristSimOffset = FVector::ZeroVector;
		LeftElbowVelocity = FVector::ZeroVector;
		LeftElbowSimOffset = FVector::ZeroVector;
		RightWristVelocity = FVector::ZeroVector;
		RightWristSimOffset = FVector::ZeroVector;
		RightElbowVelocity = FVector::ZeroVector;
		RightElbowSimOffset = FVector::ZeroVector;
		AccumulatedTime = 0.0f;
		bInitialized = true;
	}

	AccumulatedTime += DeltaTime;

	// Get spring parameters
	float SpringK, Damping;
	GetSpringDamperParams(Stiffness, SpringK, Damping);

	// Calculate asymmetry multipliers
	float LeftMultiplier = 1.0f - FMath::Max(0.0f, Asymmetry);
	float RightMultiplier = 1.0f + FMath::Min(0.0f, Asymmetry);

	// Generate noise (different seeds for each arm)
	float LeftNoise = GenerateNoise(AccumulatedTime, 0, 0.8f);
	float RightNoise = GenerateNoise(AccumulatedTime, 5000, 0.8f);

	// === Process Left Arm ===
	LeftSwingFactor = CalculateBaseSwingFactor(Phase, true);
	float LeftEffectiveSwing = ApplyRandomness(SwingAmount * LeftMultiplier, Randomness, LeftNoise);

	if (LeftWristIdx != INDEX_NONE && LeftShoulderIdx != INDEX_NONE)
	{
		FTransform LeftWristRef = Hierarchy->GetGlobalTransform(LeftWristIdx);
		FTransform LeftShoulderRef = Hierarchy->GetGlobalTransform(LeftShoulderIdx);

		FVector Forward = LeftShoulderRef.GetRotation().GetForwardVector();
		FVector Right = LeftShoulderRef.GetRotation().GetRightVector();
		FVector Up = LeftShoulderRef.GetRotation().GetUpVector();

		FVector LeftWristTargetOffset = CalculateWristTargetOffset(
			LeftSwingFactor, LeftEffectiveSwing, Stiffness, Forward, Right, Up, true);
		FVector LeftElbowTargetOffset = CalculateElbowTargetOffset(
			LeftSwingFactor, LeftEffectiveSwing, Stiffness, Right, Up, true);

		if (DeltaTime > 0.0f)
		{
			SimulateSpringDamper(LeftWristSimOffset, LeftWristVelocity, LeftWristTargetOffset, SpringK, Damping, DeltaTime);
			SimulateSpringDamper(LeftElbowSimOffset, LeftElbowVelocity, LeftElbowTargetOffset, SpringK, Damping, DeltaTime);
		}

		LeftWristTarget = LeftWristRef.GetLocation() + LeftWristSimOffset * BlendWeight;
		
		FRotator LeftRotOffset = CalculateWristRotationOffset(LeftSwingFactor, LeftEffectiveSwing, Stiffness, true);
		LeftWristRotation = LeftWristRef.GetRotation() * (LeftRotOffset * BlendWeight).Quaternion();

		if (LeftElbowIdx != INDEX_NONE)
		{
			FTransform LeftElbowRef = Hierarchy->GetGlobalTransform(LeftElbowIdx);
			LeftElbowPoleTarget = LeftElbowRef.GetLocation() + LeftElbowSimOffset * BlendWeight;
		}
	}

	// === Process Right Arm ===
	RightSwingFactor = CalculateBaseSwingFactor(Phase, false);
	float RightEffectiveSwing = ApplyRandomness(SwingAmount * RightMultiplier, Randomness, RightNoise);

	if (RightWristIdx != INDEX_NONE && RightShoulderIdx != INDEX_NONE)
	{
		FTransform RightWristRef = Hierarchy->GetGlobalTransform(RightWristIdx);
		FTransform RightShoulderRef = Hierarchy->GetGlobalTransform(RightShoulderIdx);

		FVector Forward = RightShoulderRef.GetRotation().GetForwardVector();
		FVector Right = RightShoulderRef.GetRotation().GetRightVector();
		FVector Up = RightShoulderRef.GetRotation().GetUpVector();

		FVector RightWristTargetOffset = CalculateWristTargetOffset(
			RightSwingFactor, RightEffectiveSwing, Stiffness, Forward, Right, Up, false);
		FVector RightElbowTargetOffset = CalculateElbowTargetOffset(
			RightSwingFactor, RightEffectiveSwing, Stiffness, Right, Up, false);

		if (DeltaTime > 0.0f)
		{
			SimulateSpringDamper(RightWristSimOffset, RightWristVelocity, RightWristTargetOffset, SpringK, Damping, DeltaTime);
			SimulateSpringDamper(RightElbowSimOffset, RightElbowVelocity, RightElbowTargetOffset, SpringK, Damping, DeltaTime);
		}

		RightWristTarget = RightWristRef.GetLocation() + RightWristSimOffset * BlendWeight;
		
		FRotator RightRotOffset = CalculateWristRotationOffset(RightSwingFactor, RightEffectiveSwing, Stiffness, false);
		RightWristRotation = RightWristRef.GetRotation() * (RightRotOffset * BlendWeight).Quaternion();

		if (RightElbowIdx != INDEX_NONE)
		{
			FTransform RightElbowRef = Hierarchy->GetGlobalTransform(RightElbowIdx);
			RightElbowPoleTarget = RightElbowRef.GetLocation() + RightElbowSimOffset * BlendWeight;
		}
	}

	// Debug visualization
	if (bDrawDebug && ExecuteContext.GetDrawInterface() != nullptr)
	{
		FRigVMDrawInterface* DrawInterface = ExecuteContext.GetDrawInterface();
		const FTransform& WorldToGlobal = ExecuteContext.GetToWorldSpaceTransform().Inverse();

		// Left arm (blue tones)
		DrawInterface->DrawPoint(WorldToGlobal, LeftWristTarget, 10.0f, FLinearColor::Blue);
		DrawInterface->DrawPoint(WorldToGlobal, LeftElbowPoleTarget, 6.0f, FLinearColor(0.3f, 0.5f, 1.0f));

		// Right arm (red tones)
		DrawInterface->DrawPoint(WorldToGlobal, RightWristTarget, 10.0f, FLinearColor::Red);
		DrawInterface->DrawPoint(WorldToGlobal, RightElbowPoleTarget, 6.0f, FLinearColor(1.0f, 0.5f, 0.3f));
	}
}
