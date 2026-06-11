// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/FVItemPickup.h"
#include "Inventory/FVInventoryComponent.h"
#include "FVInteractionActionData.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemPickup)

AFVItemPickup::AFVItemPickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Quantity(1)
	, HighlightColor(FLinearColor::Yellow)
	, HighlightIntensity(3.0f)
	, bIsFocused(false)
	, FloatHeight(0.0f)
	, TimeAccumulator(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.016f;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // Important for interaction traces

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionBox);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	MeshComponent->SetRenderCustomDepth(false);
	MeshComponent->SetCustomDepthStencilValue(252);

	InfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InfoWidget"));
	InfoWidgetComponent->SetupAttachment(CollisionBox);
	InfoWidgetComponent->SetVisibility(false);
	InfoWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
}

void AFVItemPickup::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();

	// Store original materials
	if (MeshComponent)
	{
		int32 NumMaterials = MeshComponent->GetNumMaterials();
		OriginalMaterials.Reserve(NumMaterials);
		
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			OriginalMaterials.Add(MeshComponent->GetMaterial(i));
		}
	}
}

void AFVItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ============================================================================
// Item Pickup API
// ============================================================================

FGameplayTag AFVItemPickup::GetInteractableTag() const
{
	return FGameplayTag();
}

FGameplayTagContainer AFVItemPickup::GetAvailableActions_Implementation() const
{
	return FGameplayTagContainer();
}

bool AFVItemPickup::ExecutePickup(UFVInventoryComponent* TargetInventory)
{
	if (!TargetInventory || !ItemData)
	{
		return false;
	}

	//if (!IsItemUsable(TargetInventory))
	//{
	//	return false;
	//}

	//if (TargetInventory->AddItem(ItemAsset, Quantity))
	//{
	//	Destroy();
	//	return true;
	//}

	return false;
}

//bool AFVItemPickupActor::IsItemUsable(UFVInventoryComponent* InventoryComponent) const
//{
//	if (!InventoryComponent || !ItemAsset)
//	{
//		return false;
//	}
//
//	return InventoryComponent->IsItemUsable(ItemAsset);
//}

// ============================================================================
// Private Methods
// ============================================================================

void AFVItemPickup::SetupVisualFeedback(bool bEnabled)
{
	if (!MeshComponent)
	{
		return;
	}

	if (bEnabled)
	{
		MeshComponent->SetRenderCustomDepth(true);
		
		// Create dynamic material instances for highlight
		int32 NumMaterials = MeshComponent->GetNumMaterials();
		DynamicMaterials.Empty(NumMaterials);
		
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			if (UMaterialInstanceDynamic* DynMat = MeshComponent->CreateDynamicMaterialInstance(i))
			{
				DynMat->SetVectorParameterValue(FName("EmissiveColor"), HighlightColor);
				DynMat->SetScalarParameterValue(FName("EmissiveIntensity"), HighlightIntensity);
				DynamicMaterials.Add(DynMat);
			}
		}
	}
	else
	{
		MeshComponent->SetRenderCustomDepth(false);

		// Restore original materials
		for (int32 i = 0; i < OriginalMaterials.Num(); ++i)
		{
			if (OriginalMaterials[i])
			{
				MeshComponent->SetMaterial(i, OriginalMaterials[i]);
			}
		}
		
		DynamicMaterials.Empty();
	}
}
