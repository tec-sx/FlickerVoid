// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/FVInventoryScreen.h"

#include "FVUITags.h"
#include "Inventory/FVUIInventoryMessageTypes.h"
#include "Inventory/FVInventorySnapshotProvider.h"
#include "Inventory/FVInventoryEntryObject.h"
#include "Components/ListView.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryScreen)

void UFVInventoryScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	ListenerHandle = Router.RegisterListener(FVUITags::UI_Inventory_Changed, this, &UFVInventoryScreen::HandleInventoryChanged);

	RefreshFromSnapshotProvider();
}

void UFVInventoryScreen::NativeOnDeactivated()
{
	if (ListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem::Get(this).UnregisterListener(ListenerHandle);
	}

	Super::NativeOnDeactivated();
}

void UFVInventoryScreen::HandleInventoryChanged(FGameplayTag Channel, const FFVUIInventoryChangedMessage& Message)
{
	RefreshFromSnapshotProvider();
}

void UFVInventoryScreen::RefreshFromSnapshotProvider()
{
	if (!ItemList)
	{
		return;
	}

	const APlayerController* PC = GetOwningPlayer();
	const IFVInventorySnapshotProvider* SnapshotProvider = PC ? Cast<IFVInventorySnapshotProvider>(PC->FindComponentByInterface(UFVInventorySnapshotProvider::StaticClass())) : nullptr;
	if (!SnapshotProvider)
	{
		return;
	}

	ItemList->ClearListItems();

	for (const TObjectPtr<UFVInventoryEntryObject>& Entry : SnapshotProvider->BuildInventorySnapshot())
	{
		ItemList->AddItem(Entry);
	}
}
