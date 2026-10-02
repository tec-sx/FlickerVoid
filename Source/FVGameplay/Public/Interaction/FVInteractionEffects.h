#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVEffect.h"
#include "GameplayTagContainer.h"
#include "FVInteractionEffects.generated.h"

USTRUCT(BlueprintType, DisplayName = "Activate Ability")
struct FLICKERVOIDGAMEPLAY_API FFVEffect_ActivateAbility : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (Categories = "Event"))
	FGameplayTag EventTag;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
