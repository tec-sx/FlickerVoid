#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_GiveItem.generated.h"

class AActor;
class UFVItemDefinition;
struct FFVItemGiveMessage;

UENUM(BlueprintType, meta = (ScriptName = "GiveDirection"))
enum class EFVGiveDirection : uint8
{
	PlayerGives UMETA(ToolTip = "The player hands the item and money to the flow's owner."),
	PlayerReceives UMETA(ToolTip = "The flow's owner hands the item and money to the player.")
};

/**
 * Hands an item and money between the player and the flow's owner, e.g. the NPC of a dialogue. Runs the player's give
 * ability (Event.Item.Give) and waits for Message.Item.GiveFinished; without a give ability it gives at once.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "FV Give Item"))
class FLICKERVOIDGAMEPLAY_API UFVFlowNode_GiveItem : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_GiveItem();

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

private:
	void HandleGiveFinished(FGameplayTag Channel, const FFVItemGiveMessage& Message);

public:
	UPROPERTY(EditAnywhere, Category = "Give")
	EFVGiveDirection Direction = EFVGiveDirection::PlayerGives;

	UPROPERTY(EditAnywhere, Category = "Give", meta = (ToolTip = "None = money only."))
	TObjectPtr<UFVItemDefinition> Item;

	UPROPERTY(EditAnywhere, Category = "Give", meta = (ClampMin = 1, EditCondition = "Item != nullptr"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, Category = "Give", meta = (ClampMin = 0))
	int32 Money = 0;

	UPROPERTY(EditAnywhere, Category = "Give", meta = (EditCondition = "Direction == EFVGiveDirection::PlayerGives", ToolTip = "Raises the receiver's relationship by their gift preferences."))
	bool bAsGift = false;

private:
	FGameplayMessageListenerHandle ListenerHandle;
	TWeakObjectPtr<AActor> Giver;
	TWeakObjectPtr<AActor> Receiver;
	bool bWaiting = false;
};
