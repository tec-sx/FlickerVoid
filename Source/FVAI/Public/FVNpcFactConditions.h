#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Facts/FVFactConditions.h"
#include "FVNpcFactConditions.generated.h"

class UFVAIConfigData;

namespace FVNpcFacts
{
	/** Fact.NPC.<NpcId>.<Aspect> for the given NPC config. Invalid if config or NpcId missing. */
	FLICKERVOIDAI_API FGameplayTag MakeTag(const UFVAIConfigData* Npc, FName Aspect);
}

/** Compares a per-NPC fact such as Met, Alive, Trust or Disposition. */
USTRUCT(BlueprintType, meta = (DisplayName = "NPC Fact"))
struct FLICKERVOIDAI_API FFVCondition_NpcFact : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVAIConfigData> Npc;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (GetOptions = "FlickerVoidAI.FVNpcFactOptions.GetAspects"))
	FName Aspect = TEXT("Met");

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "Compare != EFVFactCompare::Defined && Compare != EFVFactCompare::Undefined", EditConditionHides))
	int32 Value = 1;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** Writes a per-NPC fact. */
USTRUCT(BlueprintType, meta = (DisplayName = "NPC Fact"))
struct FLICKERVOIDAI_API FFVEffect_NpcFact : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVAIConfigData> Npc;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (GetOptions = "FlickerVoidAI.FVNpcFactOptions.GetAspects"))
	FName Aspect = TEXT("Met");

	UPROPERTY(EditAnywhere, Category = "Effect")
	EFVFactWrite Write = EFVFactWrite::Set;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (EditCondition = "Write != EFVFactWrite::Remove", EditConditionHides))
	int32 Value = 1;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};