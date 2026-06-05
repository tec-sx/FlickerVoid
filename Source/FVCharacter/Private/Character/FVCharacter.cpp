#include "Character/FVCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/FVCharacterMovementComponent.h"
#include "Character/FVCharacterTags.h"
#include "Character/FVCharacterStateManager.h"
#include "Character/FVCharacterArchetypeData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacter)

AFVCharacter::AFVCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UFVCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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
	
	// Apply archetype if configured
	if (ArchetypeData)
	{
		ApplyArchetype(ArchetypeData);
	}
}

void AFVCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update state manager (automatic tag sync)
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
	static FFVCharacterAnimationData AnimData;
	
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

//~=============================================================================
// Archetype Configuration
//~=============================================================================

void AFVCharacter::ApplyArchetype(UFVCharacterArchetypeData* NewArchetype)
{
	if (!NewArchetype || !StateManager)
	{
		return;
	}

	ArchetypeData = NewArchetype;

	// Apply all initial tags from archetype
	FGameplayTagContainer InitialTags = NewArchetype->GetAllInitialTags();
	StateManager->AddTags(InitialTags);

	// Apply ability restriction tags
	FGameplayTagContainer RestrictionTags = NewArchetype->GetAbilityRestrictionTags();
	StateManager->AddTags(RestrictionTags);

	// Apply movement config
	if (NewArchetype->MovementConfig)
	{
		UFVCharacterMovementComponent* FVMovement = GetFVCharacterMovement();
		if (FVMovement)
		{
			FVMovement->MovementConfig = NewArchetype->MovementConfig;
		}
	}

	// Apply movement modifiers
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed *= NewArchetype->MovementSpeedModifier;
		MovementComp->JumpZVelocity *= NewArchetype->JumpHeightModifier;
		MovementComp->MaxAcceleration *= NewArchetype->AccelerationModifier;
	}

	// Note: Visual properties (mesh, materials) should be applied by CharacterAppearanceComponent
	// Note: Health/Stamina should be set by AbilitySystemComponent if present
}



