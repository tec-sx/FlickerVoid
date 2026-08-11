// Fill out your copyright notice in the Description page of Project Settings.

#include "FVInventoryUIRouterComponent.h"

#include "FVItemsTags.h"
#include "FVUITags.h"
#include "Inventory/FVInventoryComponent.h"
#include "Inventory/FVInventoryMessageTypes.h"
#include "Inventory/FVUIInventoryMessageTypes.h"
#include "Inventory/FVInventoryEntryObject.h"
#include "Items/FVItemDataAsset.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryUIRouterComponent)

UFVInventoryUIRouterComponent::UFVInventoryUIRouterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }

void UFVInventoryUIRouterComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = Router.RegisterListener(FVItemsTags::Inventory_Changed, this, &UFVInventoryUIRouterComponent::HandleInventoryChanged);
}

void UFVInventoryUIRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem::Get(this).UnregisterListener(ListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInventoryUIRouterComponent::HandleInventoryChanged(FGameplayTag Channel, const FFVInventoryChangedMessage& Message)
{
	FFVUIInventoryChangedMessage UIMessage;
	UIMessage.InstanceID = Message.InstanceID;
	UIMessage.NewQuantity = Message.NewQuantity;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVUITags::UI_Inventory_Changed, UIMessage);
}

UFVInventoryComponent* UFVInventoryUIRouterComponent::GetInventoryComponent() const
{
	const APlayerController* PC = Cast<APlayerController>(GetOwner());
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UFVInventoryComponent>() : nullptr;
}

TArray<TObjectPtr<UFVInventoryEntryObject>> UFVInventoryUIRouterComponent::BuildInventorySnapshot() const
{
	TArray<TObjectPtr<UFVInventoryEntryObject>> Snapshot;

	const UFVInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		return Snapshot;
	}

	for (const FFVItemInstance& Item : InventoryComponent->GetAllItems())
	{
		UFVInventoryEntryObject* Entry = NewObject<UFVInventoryEntryObject>();
		Entry->InstanceID = Item.InstanceID;
		Entry->Quantity = Item.Quantity;

		if (Item.ItemTemplate)
		{
			Entry->DisplayName = Item.ItemTemplate->ItemData.ItemName;
			Entry->Icon = Item.ItemTemplate->ItemData.Icon;
		}

		Snapshot.Add(Entry);
	}

	return Snapshot;
}
