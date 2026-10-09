#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVKnowledge.generated.h"

class UFVKnowledgeDefinition;

UCLASS()
class FVSTORYSYSTEM_API UFVKnowledgeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Knowledge", meta = (WorldContext = "WorldContext"))
	static bool Knows(const UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge);

	UFUNCTION(BlueprintPure, Category = "Knowledge", meta = (WorldContext = "WorldContext"))
	static bool CanLearn(const UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge);

	/** Writes the fact and applies OnLearned effects. Returns false if already known or prerequisites missing. */
	UFUNCTION(BlueprintCallable, Category = "Knowledge", meta = (WorldContext = "WorldContext"))
	static bool Learn(UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge);

	UFUNCTION(BlueprintCallable, Category = "Knowledge", meta = (WorldContext = "WorldContext"))
	static void Forget(UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge);
};

USTRUCT(BlueprintType, meta = (DisplayName = "Knows"))
struct FVSTORYSYSTEM_API FFVCondition_Knows : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVKnowledgeDefinition> Knowledge;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Learn Knowledge"))
struct FVSTORYSYSTEM_API FFVEffect_Learn : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVKnowledgeDefinition> Knowledge;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
