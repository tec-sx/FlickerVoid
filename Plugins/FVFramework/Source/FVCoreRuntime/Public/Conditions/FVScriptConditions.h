#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVScriptConditions.generated.h"

/** Base for conditions written in AngelScript or Blueprint. Use it through the "Script" condition. */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FVCORERUNTIME_API UFVScriptCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition")
	bool Evaluate(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition")
	FText GetDescription() const;

	virtual UWorld* GetWorld() const override;
};

/** Base for effects written in AngelScript or Blueprint. Use it through the "Script" effect. */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FVCORERUNTIME_API UFVScriptEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "FV|Effect")
	void Apply(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Effect")
	FText GetDescription() const;

	virtual UWorld* GetWorld() const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Script"))
struct FVCORERUNTIME_API FFVCondition_Script : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TObjectPtr<UFVScriptCondition> Condition;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Script"))
struct FVCORERUNTIME_API FFVEffect_Script : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced, Category = "Effect")
	TObjectPtr<UFVScriptEffect> Effect;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
