#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FVItemEventPayload.generated.h"

class AActor;
class UFVItemDefinition;

/** Item data sent as the Optional Object of item ability events (Event.Item.*). Instigator and Target stay on the event data. */
UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVItemEventPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Inventory stack the event is about; invalid for world items."))
	FGuid InstanceId;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "World item, readable or container the event is about."))
	TObjectPtr<AActor> WorldActor;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Other character involved: give receiver or giver, pickpocket victim, merchant."))
	TObjectPtr<AActor> OtherActor;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Money handed over by a give."))
	int32 Money = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Give: the player hands over to Other Actor; otherwise Other Actor hands over to the player."))
	bool bPlayerGives = true;

	UPROPERTY(BlueprintReadOnly, Category = "Item", meta = (ToolTip = "Give: the receiver's relationship changes by their gift preferences."))
	bool bAsGift = false;
};
