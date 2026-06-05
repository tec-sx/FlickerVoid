// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/FVCharacterAnimInstance.h"
#include "Animation/FVAnimationConfigData.h"
#include "Character/FVCharacter.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

//~=============================================================================
// FFVCharacterAnimInstanceProxy Implementation
//~=============================================================================

FFVCharacterAnimInstanceProxy::FFVCharacterAnimInstanceProxy()
	: FAnimInstanceProxy()
	, CharacterData()  // Initialize with default values
	, DeltaTime(0.0f)
	, SmoothedGroundSpeed(0.0f)
	, SmoothedVelocity(FVector::ZeroVector)
	, SmoothedAcceleration(FVector::ZeroVector)
	, bIsMoving(false)
	, bHasAcceleration(false)
	, bHasVelocity(false)
	, MovementDirection(0.0f)
	, YawDelta(0.0f)
	, PreviousTransform(FTransform::Identity)
	, PreviousVelocity(FVector::ZeroVector)
{
}

FFVCharacterAnimInstanceProxy::FFVCharacterAnimInstanceProxy(UAnimInstance* InAnimInstance)
	: FAnimInstanceProxy(InAnimInstance)
	, CharacterData()  // Initialize with default values
	, DeltaTime(0.0f)
	, SmoothedGroundSpeed(0.0f)
	, SmoothedVelocity(FVector::ZeroVector)
	, SmoothedAcceleration(FVector::ZeroVector)
	, bIsMoving(false)
	, bHasAcceleration(false)
	, bHasVelocity(false)
	, MovementDirection(0.0f)
	, YawDelta(0.0f)
	, PreviousTransform(FTransform::Identity)
	, PreviousVelocity(FVector::ZeroVector)
{
}

void FFVCharacterAnimInstanceProxy::Initialize(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::Initialize(InAnimInstance);

	//~=============================================================================
	// CRITICAL: Do NOT access custom AnimInstance data here!
	//~=============================================================================
	// 
	// INITIALIZATION ORDER:
	// 1. FAnimInstanceProxy::Initialize()        ← YOU ARE HERE (too early!)
	// 2. UAnimInstance::NativeInitializeAnimation()  ← CachedCharacter/Config set here
	// 3. FAnimInstanceProxy::PreUpdate()         ← Safe to access (lazy init)
	//
	// Accessing AnimInstance->CachedCharacter or AnimInstance->AnimationConfig here
	// will result in NULL pointers because NativeInitializeAnimation() hasn't run yet!
	//
	// SOLUTION: Lazy initialization in PreUpdate() (Epic's recommended pattern)
	//~=============================================================================
}

void FFVCharacterAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	DeltaTime = DeltaSeconds;

	//~=============================================================================
	// LAZY INITIALIZATION (first frame after NativeInitializeAnimation)
	//~=============================================================================
	
	if (!CharacterPtr.IsValid() || !Config.IsValid())
	{
		// First time setup - AnimInstance is now initialized
		if (UFVCharacterAnimInstance* AnimInstance = Cast<UFVCharacterAnimInstance>(InAnimInstance))
		{
			CharacterPtr = AnimInstance->CachedCharacter;
			Config = AnimInstance->AnimationConfig;

			if (!Config.IsValid())
			{
				FV_LOG_WARNING(LogFVAnimation, "AnimInstance has no AnimationConfig assigned! Using defaults.");
			}

			if (CharacterPtr.IsValid())
			{
				UE_LOG(LogFVAnimation, Verbose, TEXT("AnimProxy initialized for character: %s"), 
					*CharacterPtr->GetName());
			}
		}
	}

	//~=============================================================================
	// GET CHARACTER DATA (GAME THREAD ONLY)
	//~=============================================================================
	
	AFVCharacter* Character = CharacterPtr.Get();
	if (!Character)
	{
		return;
	}

	// Copy character snapshot for thread-safe access in Update()
	CharacterData = Character->GetAnimationData();

	// Cache previous frame data for delta calculations
	PreviousTransform = CharacterData.ActorTransform;
	PreviousVelocity = CharacterData.Velocity;
}

void FFVCharacterAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);

	// WORKER THREAD - Only use cached CharacterData, NO direct character access!

	const UFVAnimationConfigData* ConfigPtr = Config.Get();
	if (!ConfigPtr)
	{
		return; // No config, can't compute anything
	}

	//~=============================================================================
	// Smooth Values (Interpolation)
	//~=============================================================================

	// Smooth velocity for animation
	SmoothedVelocity = FMath::VInterpTo(
		SmoothedVelocity,
		CharacterData.Velocity,
		DeltaTime,
		ConfigPtr->VelocityInterpSpeed
	);

	// Smooth acceleration
	SmoothedAcceleration = FMath::VInterpTo(
		SmoothedAcceleration,
		CharacterData.InputAcceleration,
		DeltaTime,
		ConfigPtr->AccelerationInterpSpeed
	);

	// Smooth ground speed
	SmoothedGroundSpeed = FMath::FInterpTo(
		SmoothedGroundSpeed,
		CharacterData.Velocity.Size2D(),
		DeltaTime,
		ConfigPtr->VelocityInterpSpeed
	);

	//~=============================================================================
	// Movement State Calculations
	//~=============================================================================

	// Is moving?
	bIsMoving = SmoothedGroundSpeed > ConfigPtr->MovingSpeedThreshold;

	// Has acceleration?
	bHasAcceleration = !CharacterData.InputAcceleration.IsNearlyZero(1.0f);

	// Has velocity?
	bHasVelocity = !CharacterData.Velocity.IsNearlyZero(1.0f);

	//~=============================================================================
	// Movement Direction (relative to character rotation)
	//~=============================================================================

	if (bHasVelocity && bIsMoving)
	{
		// Calculate angle between velocity and forward vector
		FVector Velocity2D = CharacterData.Velocity.GetSafeNormal2D();
		FVector Forward2D = CharacterData.ActorTransform.GetRotation().GetForwardVector().GetSafeNormal2D();

		// Get angle (-180 to 180)
		float Angle = FMath::RadiansToDegrees(FMath::Atan2(
			FVector::CrossProduct(Forward2D, Velocity2D).Z,
			FVector::DotProduct(Forward2D, Velocity2D)
		));

		// Convert to 0-360 range
		MovementDirection = Angle < 0.0f ? Angle + 360.0f : Angle;
	}
	else
	{
		MovementDirection = 0.0f;
	}

	//~=============================================================================
	// Rotation Delta (for turn-in-place)
	//~=============================================================================

	if (DeltaTime > SMALL_NUMBER)
	{
		FRotator CurrentRotation = CharacterData.ActorTransform.GetRotation().Rotator();
		FRotator PreviousRotation = PreviousTransform.GetRotation().Rotator();

		YawDelta = FMath::FindDeltaAngleDegrees(PreviousRotation.Yaw, CurrentRotation.Yaw);
	}
	else
	{
		YawDelta = 0.0f;
	}
}

//~=============================================================================
// UFVCharacterAnimInstance Implementation
//~=============================================================================

UFVCharacterAnimInstance::UFVCharacterAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AnimationConfig(nullptr)
	, CachedCharacter(nullptr)
{
}

FAnimInstanceProxy* UFVCharacterAnimInstance::CreateAnimInstanceProxy()
{
	return &Proxy;
}

void UFVCharacterAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	// Proxy is a member variable, don't delete it
}

void UFVCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cache character reference
	AActor* OwningActor = TryGetPawnOwner();
	if (!OwningActor)
	{
		FV_LOG_ERROR(LogFVAnimation, "AnimInstance has no owning actor!");
		return;
	}

	CachedCharacter = Cast<AFVCharacter>(OwningActor);
	if (!CachedCharacter)
	{
		FV_LOG_ERROR(LogFVAnimation, "Owning actor is not AFVCharacter! Animation system will not function.");
		return;
	}

	// Validate configuration
	if (!AnimationConfig)
	{
		FV_LOG_WARNING(LogFVAnimation, "No AnimationConfig assigned to AnimInstance on %s. Using default values.",
			*CachedCharacter->GetName());
		
		// Create default config on the fly (not ideal, but prevents crashes)
		AnimationConfig = NewObject<UFVAnimationConfigData>(this);
	}

	UE_LOG(LogFVAnimation, Log, TEXT("AnimInstance initialized for character: %s"), *CachedCharacter->GetName());
}

void UFVCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UFVCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter)
	{
		return;
	}

	// Detect tag changes (for reactive Blueprint logic)
	const FGameplayTagContainer& CurrentTags = Proxy.CharacterData.CharacterTags;
	
	// Find added tags
	for (const FGameplayTag& Tag : CurrentTags)
	{
		if (!PreviousFrameTags.HasTag(Tag))
		{
			// Tag added
			OnTagChanged(Tag, true);
		}
	}

	// Find removed tags
	for (const FGameplayTag& Tag : PreviousFrameTags)
	{
		if (!CurrentTags.HasTag(Tag))
		{
			// Tag removed
			OnTagChanged(Tag, false);
		}
	}

	// Update previous frame cache
	PreviousFrameTags = CurrentTags;

	// Call Blueprint event
	OnCharacterDataUpdated();
}
