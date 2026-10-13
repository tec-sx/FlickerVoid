#include "UI/FVEquipmentSlotWidget.h"

#include "Equipment/FVEquipmentComponent.h"
#include "GameFramework/Pawn.h"
#include "Inventory/FVInventoryComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVEquipmentSlotWidget)

void UFVEquipmentSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsDesignTime())
	{
		ClearItem();
		OnSlotChanged(nullptr);
	}
}

void UFVEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Bind(EquipmentOverride.IsValid() ? EquipmentOverride.Get() : UFVEquipmentComponent::Find(GetOwningPlayerPawn()));
}

void UFVEquipmentSlotWidget::NativeDestruct()
{
	Unbind();
	Super::NativeDestruct();
}

void UFVEquipmentSlotWidget::SetEquipment(UFVEquipmentComponent* InEquipment)
{
	EquipmentOverride = InEquipment;
	Bind(InEquipment ? InEquipment : UFVEquipmentComponent::Find(GetOwningPlayerPawn()));
}

UFVItemDefinition* UFVEquipmentSlotWidget::GetBlockingItem() const
{
	const UFVEquipmentComponent* Bound = Equipment.Get();
	return Bound && !Bound->GetEquipped(EquipmentSlot) ? Bound->GetBlockingItem(EquipmentSlot) : nullptr;
}

void UFVEquipmentSlotWidget::Refresh()
{
	const UFVEquipmentComponent* Bound = Equipment.Get();
	UFVItemDefinition* Worn = Bound ? Bound->GetEquipped(EquipmentSlot) : nullptr;

	// The carried stack when there is one, so instance data (dye, wear) shows too.
	FFVItemStack Stack;
	if (Worn)
	{
		const FGuid InstanceId = Bound->GetEquippedInstance(EquipmentSlot);
		const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(Bound->GetOwner());
		if (const FFVItemStack* Carried = Inventory ? Inventory->FindStack(InstanceId) : nullptr)
		{
			Stack = *Carried;
		}
		else
		{
			Stack.InstanceId = InstanceId;
			Stack.Item = Worn;
		}
	}

	SetItem(Stack);
	OnSlotChanged(GetBlockingItem());
}

void UFVEquipmentSlotWidget::Bind(UFVEquipmentComponent* InEquipment)
{
	Unbind();
	Equipment = InEquipment;

	if (InEquipment)
	{
		InEquipment->OnEquipmentChanged.AddUniqueDynamic(this, &ThisClass::HandleEquipmentChanged);
		InEquipment->OnEquipmentReset.AddUniqueDynamic(this, &ThisClass::HandleEquipmentReset);
	}
	Refresh();
}

void UFVEquipmentSlotWidget::Unbind()
{
	if (UFVEquipmentComponent* Bound = Equipment.Get())
	{
		Bound->OnEquipmentChanged.RemoveAll(this);
		Bound->OnEquipmentReset.RemoveAll(this);
	}
	Equipment = nullptr;
}

void UFVEquipmentSlotWidget::HandleEquipmentChanged(FGameplayTag ChangedSlot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem)
{
	// Any change can cover or uncover this slot, not only one in it.
	Refresh();
}

void UFVEquipmentSlotWidget::HandleEquipmentReset()
{
	Refresh();
}
