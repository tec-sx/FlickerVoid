// Copyright FlickerVoid. All Rights Reserved.

#include "RigUnits/FVRigUnit_HipSwing.h"
#include "Units/RigUnitContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRigUnit_HipSwing)

FFVRigUnit_HipSwing_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	// Early out if disabled
	if (BlendWeight <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	// Validate pelvis
	if (PelvisBone.IsNone())
	{
		return;
	}

	const FRigElementKey PelvisKey(PelvisBone, ERigElementType::Bone);
	if (!CachedPelvis.UpdateCache(PelvisKey, Hierarchy))
	{
		return;
	}

	// Cache foot and spine bones
	auto CacheBone = [Hierarchy](const FName& BoneName, FCachedRigElement& Cache) -> bool
	{
		if (BoneName.IsNone()) return false;
		const FRigElementKey Key(BoneName, ERigElementType::Bone);
		return Cache.UpdateCache(Key, Hierarchy);
	};

	CacheBone(LeftFootBone, CachedLeftFoot);
	CacheBone(RightFootBone, CachedRightFoot);
	CacheBone(Spine01Bone, CachedSpine01);
	CacheBone(Spine02Bone, CachedSpine02);
	CacheBone(Spine03Bone, CachedSpine03);

	// ========================================================================
	// CAPTURE FOOT POSITIONS BEFORE PELVIS MODIFICATION
	// These become our IK effector targets (output to TwoBoneIK nodes)
	// ========================================================================
	if (CachedLeftFoot.IsValid())
	{
		LeftFootEffector = Hierarchy->GetGlobalTransform(CachedLeftFoot);
	}
	if (CachedRightFoot.IsValid())
	{
		RightFootEffector = Hierarchy->GetGlobalTransform(CachedRightFoot);
	}

	// ========================================================================
	// PELVIS MODIFICATION
	// ========================================================================
	FTransform PelvisTransform = Hierarchy->GetGlobalTransform(CachedPelvis);
	const FVector CurrentPosition = PelvisTransform.GetLocation();
	const FQuat CurrentRotation = PelvisTransform.GetRotation();

	// Initialize reference pose on first run
	if (!bInitialized)
	{
		InitialPelvisRotation = CurrentRotation;
		InitialPelvisPosition = CurrentPosition;
		bInitialized = true;
	}

	// Calculate rotation and position deltas from initial pose
	FQuat RotationDelta = InitialPelvisRotation.Inverse() * CurrentRotation;
	FVector PositionDelta = CurrentPosition - InitialPelvisPosition;

	// Scale the deltas by SwingAmount
	FQuat ScaledRotationDelta = FQuat::Slerp(FQuat::Identity, RotationDelta, SwingAmount);
	FVector ScaledPositionDelta = PositionDelta;
	ScaledPositionDelta.Z *= VerticalBobAmount;

	// Calculate new pelvis transform
	FQuat NewPelvisRotation = InitialPelvisRotation * ScaledRotationDelta;
	FVector NewPelvisPosition = InitialPelvisPosition + ScaledPositionDelta;

	// Blend with current based on weight
	if (BlendWeight < 1.0f - KINDA_SMALL_NUMBER)
	{
		NewPelvisRotation = FQuat::Slerp(CurrentRotation, NewPelvisRotation, BlendWeight);
		NewPelvisPosition = FMath::Lerp(CurrentPosition, NewPelvisPosition, BlendWeight);
	}

	// Calculate the pelvis rotation change for spine compensation
	FQuat PelvisRotationChange = CurrentRotation.Inverse() * NewPelvisRotation;

	// Apply pelvis modification
	FTransform NewPelvisTransform = PelvisTransform;
	NewPelvisTransform.SetRotation(NewPelvisRotation);
	NewPelvisTransform.SetLocation(NewPelvisPosition);
	Hierarchy->SetGlobalTransform(CachedPelvis, NewPelvisTransform, true);

	// ========================================================================
	// SPINE COUNTER-ROTATION
	// Distributes counter-rotation across spine to keep upper body stable
	// ========================================================================
	if (SpineFlexibility > KINDA_SMALL_NUMBER)
	{
		FQuat CounterRotation = FQuat::Slerp(FQuat::Identity, PelvisRotationChange.Inverse(), SpineFlexibility * BlendWeight);

		// Count valid spine bones
		int32 SpineCount = 0;
		if (CachedSpine01.IsValid()) SpineCount++;
		if (CachedSpine02.IsValid()) SpineCount++;
		if (CachedSpine03.IsValid()) SpineCount++;

		if (SpineCount > 0)
		{
			// Lower spine gets more rotation, upper gets less (more natural)
			float Weights[3] = { 0.5f, 0.33f, 0.17f };
			float TotalWeight = 0.0f;
			for (int32 i = 0; i < SpineCount; i++) TotalWeight += Weights[i];

			auto ApplySpineRotation = [&](FCachedRigElement& SpineCache, float Weight)
			{
				if (!SpineCache.IsValid()) return;

				float NormalizedWeight = Weight / TotalWeight;
				FQuat SpineCounterRot = FQuat::Slerp(FQuat::Identity, CounterRotation, NormalizedWeight);

				FTransform SpineTransform = Hierarchy->GetGlobalTransform(SpineCache);
				FQuat NewSpineRot = SpineTransform.GetRotation() * SpineCounterRot;
				SpineTransform.SetRotation(NewSpineRot);
				Hierarchy->SetGlobalTransform(SpineCache, SpineTransform, true);
			};

			int32 WeightIdx = 0;
			if (CachedSpine01.IsValid()) ApplySpineRotation(CachedSpine01, Weights[WeightIdx++]);
			if (CachedSpine02.IsValid()) ApplySpineRotation(CachedSpine02, Weights[WeightIdx++]);
			if (CachedSpine03.IsValid()) ApplySpineRotation(CachedSpine03, Weights[WeightIdx++]);
		}
	}
}
