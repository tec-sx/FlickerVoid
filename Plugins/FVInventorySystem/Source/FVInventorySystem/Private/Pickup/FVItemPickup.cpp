#include "Pickup/FVItemPickup.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Pickup/FVPickupComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemPickup)

AFVItemPickup::AFVItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	RootComponent = Mesh;

	Pickup = CreateDefaultSubobject<UFVPickupComponent>(TEXT("Pickup"));
}

void AFVItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyItemMesh();
}

void AFVItemPickup::BeginPlay()
{
	Super::BeginPlay();
	Pickup->OnItemChanged.AddUniqueDynamic(this, &AFVItemPickup::ApplyItemMesh);
	ApplyItemMesh();
}

void AFVItemPickup::ApplyItemMesh()
{
	const UFVItemDefinition* Item = Pickup ? Pickup->GetItem() : nullptr;
	const FFVItemFragment_Pickup* PickupData = Item ? Item->FindFragment<FFVItemFragment_Pickup>() : nullptr;
	if (PickupData && !PickupData->WorldMesh.IsNull())
	{
		Mesh->SetStaticMesh(PickupData->WorldMesh.LoadSynchronous());
	}
}
