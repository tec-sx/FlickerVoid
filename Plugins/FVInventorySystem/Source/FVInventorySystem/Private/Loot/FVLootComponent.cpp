#include "Loot/FVLootComponent.h"

#include "FVInventorySystem.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVInventoryComponent.h"
#include "Loot/FVLootLibrary.h"
#include "Loot/FVLootTableDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVLootComponent)

bool UFVLootComponent::RollIfNeeded(AActor* Instigator)
{
	if (bRolled || !Table)
	{
		return false;
	}

	UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
	if (!Inventory)
	{
		UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: loot has no inventory component to fill."), *GetNameSafe(GetOwner()));
		return false;
	}

	bRolled = true;
	UFVLootLibrary::GiveLoot(Table, Inventory, Instigator);
	return true;
}
