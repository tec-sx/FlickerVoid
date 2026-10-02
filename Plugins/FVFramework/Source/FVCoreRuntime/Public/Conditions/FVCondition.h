#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "FVCondition.generated.h"

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVConditionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Condition")
	TObjectPtr<UObject> WorldContext = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Condition")
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Condition")
	TObjectPtr<AActor> Target = nullptr;
};

USTRUCT(BlueprintType, meta = (Hidden))
struct FVCORERUNTIME_API FFVConditionBase
{
	GENERATED_BODY()

	virtual ~FFVConditionBase() = default;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	bool Evaluate(const FFVConditionContext& Context) const { return EvaluateImpl(Context) != bInvert; }
	virtual FText GetDescription() const { return FText::GetEmpty(); }

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const { return true; }
};

UENUM(BlueprintType)
enum class EFVConditionMode : uint8
{
	All,
	Any
};

UENUM(BlueprintType)
enum class EFVConditionFailurePresentation : uint8
{
	Hidden,
	ShowLocked,
	ShowLockedWithReason
};

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVConditionSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EFVConditionMode Mode = EFVConditionMode::All;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVConditionBase>> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EFVConditionFailurePresentation FailurePresentation = EFVConditionFailurePresentation::Hidden;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (EditCondition = "FailurePresentation == EFVConditionFailurePresentation::ShowLockedWithReason"))
	FText FailureReason;

	bool IsEmpty() const { return Conditions.IsEmpty(); }
	bool Evaluate(const FFVConditionContext& Context) const;
	FText GetDescription() const;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Group"))
struct FVCORERUNTIME_API FFVCondition_Group : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	FFVConditionSet Set;

	virtual FText GetDescription() const override { return Set.GetDescription(); }

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override { return Set.Evaluate(Context); }
};
