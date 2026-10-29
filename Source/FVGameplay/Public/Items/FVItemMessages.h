#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVItemMessages.generated.h"

class AActor;
class UFVItemDefinition;
class UFVKnowledgeDefinition;

/** Message.Item.Read: a note or document was read. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVItemReadMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<AActor> Reader = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "The note itself."))
	TObjectPtr<UFVKnowledgeDefinition> Knowledge = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "The readable actor in the world."))
	TObjectPtr<AActor> Source = nullptr;
};

/** Message.Item.GiveFinished: a give ended. Always sent, also on failure or cancel. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVItemGiveMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<AActor> Giver = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<AActor> Receiver = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "None when only money was given."))
	TObjectPtr<UFVItemDefinition> Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Money = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "The item and money changed hands."))
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "Given as a gift: the receiver reacts by their gift preferences."))
	bool bAsGift = false;
};

/** Message.Item.Stolen: something was stolen, or a theft was noticed. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVItemStolenMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<AActor> Thief = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "Character or container stolen from."))
	TObjectPtr<AActor> Victim = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "None when only money was taken, or nothing was."))
	TObjectPtr<UFVItemDefinition> Item = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	int32 Money = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "Faction or character Id owning what was taken; empty = unknown owner."))
	FGameplayTag Owner;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "The victim noticed the attempt."))
	bool bNoticed = false;
};

/** Message.Item.Noise: a thrown item made noise. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVItemNoiseMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "Who threw the item."))
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	float Loudness = 1.f;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (Units = "cm"))
	float Range = 0.f;
};

/** Message.Item.ContainerOpened and Message.Item.TradeRequested: someone opened a container or asked a merchant to trade. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVItemContainerMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<AActor> Opener = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Item", meta = (ToolTip = "The container, or the merchant."))
	TObjectPtr<AActor> Container = nullptr;
};
