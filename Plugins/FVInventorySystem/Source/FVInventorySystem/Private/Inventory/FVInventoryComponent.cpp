#include "Inventory/FVInventoryComponent.h"

#include "Conditions/FVConditionStatics.h"
#include "GameFramework/Actor.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemFragments.h"
#include "Pickup/FVItemReceiverComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventoryComponent)

UFVInventoryComponent::UFVInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVInventoryComponent* UFVInventoryComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVInventoryComponent>() : nullptr;
}

void UFVInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnItemReceived.AddUniqueDynamic(this, &UFVInventoryComponent::HandleItemReceived);
	}
}

void UFVInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVItemReceiverComponent* Receiver = UFVItemReceiverComponent::FindReceiver(GetOwner()))
	{
		Receiver->OnItemReceived.RemoveDynamic(this, &UFVInventoryComponent::HandleItemReceived);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInventoryComponent::HandleItemReceived(UFVItemDefinition* Item, const int32 Quantity, AActor* Source)
{
	AddItem(Item, Quantity);
}

int32 UFVInventoryComponent::AddItem(UFVItemDefinition* Item, const int32 Quantity)
{
	if (!Item || Quantity <= 0)
	{
		return 0;
	}

	const int32 Current = GetQuantity(Item);
	const int32 Room = Item->MaxQuantity > 0 ? FMath::Max(Item->MaxQuantity - Current, 0) : Quantity;
	const int32 Added = FMath::Min(Quantity, Room);
	if (Added > 0)
	{
		SetQuantity(Item, Current + Added);
	}
	return Added;
}

bool UFVInventoryComponent::RemoveItem(UFVItemDefinition* Item, const int32 Quantity)
{
	const int32 Current = GetQuantity(Item);
	if (!Item || Quantity <= 0 || Current < Quantity)
	{
		return false;
	}

	SetQuantity(Item, Current - Quantity);
	return true;
}

int32 UFVInventoryComponent::GetQuantity(const UFVItemDefinition* Item) const
{
	const FFVItemStack* Stack = Items.FindByPredicate([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
	return Stack ? Stack->Quantity : 0;
}

TArray<FFVItemStack> UFVInventoryComponent::GetItemsInCategory(const FGameplayTag Category) const
{
	return Items.FilterByPredicate([&Category](const FFVItemStack& Entry)
	{
		return Entry.Item && Entry.Item->Category.MatchesTag(Category);
	});
}

bool UFVInventoryComponent::CanUseItem(const UFVItemDefinition* Item) const
{
	const FFVItemFragment_Usable* Usable = Item ? Item->FindFragment<FFVItemFragment_Usable>() : nullptr;
	if (!Usable || !HasItem(Item))
	{
		return false;
	}
	return Usable->UseConditions.Evaluate(UFVConditionStatics::MakeContext(GetOwner(), GetOwner()));
}

bool UFVInventoryComponent::UseItem(UFVItemDefinition* Item)
{
	if (!CanUseItem(Item))
	{
		return false;
	}

	const FFVItemFragment_Usable* Usable = Item->FindFragment<FFVItemFragment_Usable>();
	Usable->Effects.Apply(UFVConditionStatics::MakeContext(GetOwner(), GetOwner()));
	if (Usable->bConsumeOnUse)
	{
		RemoveItem(Item, 1);
	}

	OnItemUsed.Broadcast(Item);
	return true;
}

FFVItemStack* UFVInventoryComponent::FindStack(const UFVItemDefinition* Item)
{
	return Items.FindByPredicate([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
}

void UFVInventoryComponent::SetQuantity(UFVItemDefinition* Item, const int32 NewQuantity)
{
	FFVItemStack* Stack = FindStack(Item);
	const int32 OldQuantity = Stack ? Stack->Quantity : 0;
	if (OldQuantity == NewQuantity)
	{
		return;
	}

	if (NewQuantity <= 0)
	{
		Items.RemoveAll([Item](const FFVItemStack& Entry) { return Entry.Item == Item; });
	}
	else if (Stack)
	{
		Stack->Quantity = NewQuantity;
	}
	else
	{
		FFVItemStack& NewStack = Items.AddDefaulted_GetRef();
		NewStack.Item = Item;
		NewStack.Quantity = NewQuantity;
	}

	OnItemChanged.Broadcast(Item, OldQuantity, FMath::Max(NewQuantity, 0));
}
