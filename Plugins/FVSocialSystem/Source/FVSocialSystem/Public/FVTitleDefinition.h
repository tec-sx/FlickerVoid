#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "FVTitleDefinition.generated.h"

/**
 * A name the world gives the player once its conditions hold, e.g. "the one who robbed the docks".
 * Id is the fact that records it, so dialogue and quests can test it like any other fact.
 */
UCLASS(BlueprintType)
class FVSOCIALSYSTEM_API UFVTitleDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Evaluated whenever a fact changes; usually fame, notoriety or a faction standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title")
	FFVConditionSet Conditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title")
	FFVEffectList OnEarned;

	/** Titles that fall away once this one is earned, e.g. a lesser version of the same story. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title")
	TArray<TObjectPtr<UFVTitleDefinition>> Replaces;

	/** A title the player can lose again when its conditions stop holding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title")
	bool bTransient = false;
};
