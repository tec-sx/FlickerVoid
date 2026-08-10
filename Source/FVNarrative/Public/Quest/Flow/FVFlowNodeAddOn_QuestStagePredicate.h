#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "FactDB/FVFactTypes.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Quest/FVQuestFactHelpers.h"
#include "FVFlowNodeAddOn_QuestStagePredicate.generated.h"

/**
 * Sugar over Fact Predicate for the common quest test:
 * Fact.Quest.<ChapterId>.<QuestId>.Stage <Operator> <Stage>.
 *
 * Because stage values are sparse and ordered, GreaterOrEqual expresses
 * "this quest has at least reached that point".
 */
UCLASS(MinimalApi, NotBlueprintable, meta = (DisplayName = "Quest Stage Predicate"))
class UFVFlowNodeAddOn_QuestStagePredicate : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	UFVFlowNodeAddOn_QuestStagePredicate();

	virtual bool EvaluatePredicate_Implementation() const override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

protected:
	/** Must be an existing Fact.Quest.<ChapterId>.<QuestId>.Stage tag. */
	UPROPERTY(EditAnywhere, Category = "Quest", meta = (Categories = "Fact.Quest"))
	FGameplayTag QuestStage;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVFactCompareOperator Operator = EFVFactCompareOperator::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Quest", meta = (EditCondition = "Operator != EFVFactCompareOperator::IsDefined && Operator != EFVFactCompareOperator::IsUndefined"))
	EFVQuestStage Stage = EFVQuestStage::Accepted;

private:
	FFVFactCondition MakeCondition() const;
};
