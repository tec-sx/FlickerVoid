#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "FactDB/FVFactTypes.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Quest/FVQuestFactHelpers.h"
#include "FVFlowNodeAddOn_ChapterStagePredicate.generated.h"

/**
 * Sugar over Fact Predicate for chapter level gating:
 * Fact.Chapter.<ChapterId>.Stage <Operator> <Stage>.
 */
UCLASS(MinimalApi, NotBlueprintable, meta = (DisplayName = "Chapter Stage Predicate"))
class UFVFlowNodeAddOn_ChapterStagePredicate : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	UFVFlowNodeAddOn_ChapterStagePredicate();

	virtual bool EvaluatePredicate_Implementation() const override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "Chapter", meta = (Categories = "Fact.Chapter"))
	FGameplayTag ChapterStage;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	EFVFactCompareOperator Operator = EFVFactCompareOperator::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Chapter", meta = (EditCondition = "Operator != EFVFactCompareOperator::IsDefined && Operator != EFVFactCompareOperator::IsUndefined"))
	EFVQuestStage Stage = EFVQuestStage::InProgress;

private:
	FFVFactCondition MakeCondition() const;
};
