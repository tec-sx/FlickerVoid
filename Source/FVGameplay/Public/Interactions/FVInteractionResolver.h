#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/FVInteractionActionInfo.h"
#include "FVInteractionResolver.generated.h"

class UFVInteractionAction;
class UFVInteractionTargetComponent;

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVResolvedInteraction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UFVInteractionAction> Action = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVInteractionActionInfo Info;

	bool IsBound() const { return Action != nullptr; }
};


USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVResolvedInteractionSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<FFVResolvedInteraction> Slots;

	FFVResolvedInteractionSet()
	{
		Slots.SetNum(static_cast<int32>(EFVInteractionSlot::MAX));
	}

	const FFVResolvedInteraction& GetSlot(EFVInteractionSlot Slot) const
	{
		return Slots[static_cast<int32>(Slot)];
	}

	bool HasAnyBinding() const
	{
		return Slots.ContainsByPredicate([](const FFVResolvedInteraction& Entry) { return Entry.IsBound(); });
	}
};


UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionResolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static FFVResolvedInteractionSet ResolveInteractions(
		UFVInteractionTargetComponent* Target,
		AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static FFVResolvedInteraction ResolveSlot(
		UFVInteractionTargetComponent* Target,
		AActor* Instigator,
		EFVInteractionSlot Slot);
};
