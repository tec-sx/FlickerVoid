#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "StructUtils/InstancedStruct.h"
#include "FVEffect.generated.h"

USTRUCT(BlueprintType, meta = (Hidden))
struct FVCORERUNTIME_API FFVEffectBase
{
	GENERATED_BODY()

	virtual ~FFVEffectBase() = default;

	virtual void Apply(const FFVConditionContext& Context) const {}
	virtual FText GetDescription() const { return FText::GetEmpty(); }
};

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVEffectList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVEffectBase>> Effects;

	bool IsEmpty() const { return Effects.IsEmpty(); }
	void Apply(const FFVConditionContext& Context) const;
	FText GetDescription() const;
};
