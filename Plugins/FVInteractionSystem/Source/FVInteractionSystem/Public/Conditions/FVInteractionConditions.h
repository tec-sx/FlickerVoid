#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "GameplayTagContainer.h"

#include "FVInteractionConditions.generated.h"

USTRUCT(BlueprintType, meta = (DisplayName = "Interactor Tags"))
struct FVINTERACTIONSYSTEM_API FFVCondition_InteractorTags : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (Tooltip = "Require every tag in RequiredTags instead of any one of them."))
	bool bRequireAll = true;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};
