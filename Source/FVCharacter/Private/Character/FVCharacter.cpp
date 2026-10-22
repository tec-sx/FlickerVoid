#include "Character/FVCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Engine/CollisionProfile.h"
#include "FlowComponent.h"
#include "FVCoreTags.h"
#include "Movement/FVLocomotionComponent.h"
#include "Movement/FVMovementSetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacter)

AFVCharacter::AFVCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetReplicatingMovement(false);

	// Same subobject names as ACharacter, so Blueprints made on the old base keep their capsule and mesh settings.
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCylinder"));
	CapsuleComponent->InitCapsuleSize(34.f, 88.f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh0"));
	Mesh->SetupAttachment(CapsuleComponent);
	Mesh->AlwaysLoadOnClient = true;
	Mesh->AlwaysLoadOnServer = true;
	Mesh->bOwnerNoSee = false;
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	Mesh->bCastDynamicShadow = true;
	Mesh->bAffectDynamicIndirectLighting = true;
	Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCanEverAffectNavigation(false);

	Mover = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("CharacterMover"));
	Mover->bGatherInputFromAllInputProducerComponents = true;
	Locomotion = CreateDefaultSubobject<UFVLocomotionComponent>(TEXT("Locomotion"));
	MovementSets = CreateDefaultSubobject<UFVMovementSetComponent>(TEXT("MovementSets"));

	FlowComponent = CreateDefaultSubobject<UFlowComponent>(TEXT("FlowComponent"));
	FlowComponent->IdentityTags = FGameplayTagContainer(FVCoreTags::Player_Pawn);
}

void AFVCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Editor worlds keep the Blueprint's modes; changing them on placed instances can stop those pawns from moving.
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		MovementSets->InstallStartingSets();
	}
}

void AFVCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Mover->SetPrimaryVisualComponent(Mesh);
}

FVector AFVCharacter::GetVelocity() const
{
	return Mover ? Mover->GetVelocity() : Super::GetVelocity();
}

FGameplayTagContainer AFVCharacter::GetIdentityTags() const
{
	return GetFlowComponent() ? GetFlowComponent()->IdentityTags : FGameplayTagContainer();
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
