#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/FVInteractionInfo.h"
#include "FVInteractionResolver.generated.h"

class UFVInteractionConfig;
class UFVInteractionTargetComponent;

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVResolvedInteraction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UFVInteractionConfig> Config = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVInteractionInfo Info;

	bool IsBound() const { return Config != nullptr; }
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
