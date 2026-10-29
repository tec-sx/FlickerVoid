#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVLootComponent.generated.h"

class UFVLootTableDefinition;

/** Fills the UFVInventoryComponent on the same actor from a loot table, once, e.g. when a container is first opened. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVINVENTORYSYSTEM_API UFVLootComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVLootComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "FV|Loot", meta = (ToolTip = "Rolls Table into the owner's inventory if it hasn't been rolled yet. Entry conditions see Instigator and the owner as Target. Returns true if it rolled now."))
	bool RollIfNeeded(AActor* Instigator);

	UFUNCTION(BlueprintPure, Category = "FV|Loot")
	bool HasRolled() const { return bRolled; }

	UFUNCTION(BlueprintPure, Category = "FV|Loot", meta = (ToolTip = "Faction or character Id owning the contents; empty = free to take."))
	FGameplayTag GetOwnerTag() const { return OwnerTag; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UFVLootTableDefinition> Table;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ToolTip = "Faction or character Id owning the contents; items taken get the Stolen mark. Empty = free to take."))
	FGameplayTag OwnerTag;

private:
	UPROPERTY(SaveGame)
	bool bRolled = false;
};
