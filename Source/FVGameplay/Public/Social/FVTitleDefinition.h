#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "FVTitleDefinition.generated.h"

/**
 * A name the world gives the player once its conditions hold, e.g. "the one who robbed the docks".
 * Earned titles are kept for good; Id is the fact that records the title's place in that history (0 = not earned).
 */
UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVTitleDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title", meta = (ToolTip = "Evaluated whenever a fact changes; usually fame, notoriety or a faction standing scale. Their at-least thresholds rank the title: the hardest held title is current."))
	FFVConditionSet Conditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title", meta = (ToolTip = "Applied once, the first time the title is earned."))
	FFVEffectList OnEarned;

protected:
	virtual bool RequiresId() const override { return true; }
};
