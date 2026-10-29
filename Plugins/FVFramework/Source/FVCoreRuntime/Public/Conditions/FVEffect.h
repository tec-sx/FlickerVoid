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

/**
 * A change applied with a context, written in C++ or AngelScript. Instanced into the asset that uses it and shared by
 * every caller, so it writes its result to the world (facts, components) and never to its own members. Until effect
 * lists hold these directly, pick them through the Object effect.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FVCORERUNTIME_API UFVEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "FV|Effect")
	void Apply(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Effect")
	FText GetDescription() const;

	virtual UWorld* GetWorld() const override;
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

/** Holds a UFVEffect (C++ or AngelScript) in an effect list until lists hold them directly. */
USTRUCT(BlueprintType, meta = (DisplayName = "Object"))
struct FVCORERUNTIME_API FFVEffect_Object : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced, Category = "Effect")
	TObjectPtr<UFVEffect> Effect;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
