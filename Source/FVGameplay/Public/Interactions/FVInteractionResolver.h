#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/FVInteractionActionInfo.h"
#include "FVInteractionResolver.generated.h"

class UFVInteractionAction;
class UFVInteractionTargetComponent;

/**
 * A single resolved slot binding for a focused target.
 * Carries both the runtime action (for execution) and its display info (for UI),
 * so the ability and the UI never filter independently.
 */
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

/**
 * The resolved state of every input slot for one target/instigator pair.
 * Indexed by EFVInteractionSlot; always sized to EFVInteractionSlot::MAX.
 */
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

/**
 * The single source of truth for "which actions are on which slot, and are they available".
 * Both the interact ability and the UI router consume this; neither filters on its own.
 */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionResolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Resolves every slot for the given target against the instigator's owned tags. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static FFVResolvedInteractionSet ResolveInteractions(
		UFVInteractionTargetComponent* Target,
		const FGameplayTagContainer& InstigatorTags);

	/** Resolves a single slot. Returns an unbound entry when nothing occupies it. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static FFVResolvedInteraction ResolveSlot(
		UFVInteractionTargetComponent* Target,
		const FGameplayTagContainer& InstigatorTags,
		EFVInteractionSlot Slot);
};
