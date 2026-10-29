#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCompare.h"
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

	/** The target or the instigator, for a condition or effect that needs it; warns outside Shipping, naming the Requester (struct or class), when it's missing. */
	AActor* GetActor(bool bTarget, const UStruct* Requester) const;
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

	/** Scores an at-least condition (fame >= 80) to rank what it unlocks; false for other conditions and inverted ones. */
	bool GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const { return !bInvert && GetScoreImpl(Context, OutScore); }

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const { return true; }
	virtual bool GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const { return false; }
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

/**
 * A rule checked against a context, written in C++ or AngelScript. Instanced into the asset that uses it and shared by
 * every caller, so it reads only its properties and the context and keeps no state. Until condition sets hold these
 * directly, pick them through the Object condition.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FVCORERUNTIME_API UFVCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Condition", meta = (ToolTip = "Evaluate's result, flipped when Invert is set."))
	bool Check(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Condition", meta = (ToolTip = "Ranks what an at-least condition unlocks; false when the condition isn't scored or is inverted."))
	bool GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition", meta = (ToolTip = "Whether the condition holds, before Invert. Override it."))
	bool Evaluate(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition", meta = (ToolTip = "Override for at-least conditions (fame >= 80): fill the score and return true. Default: not scored."))
	bool Score(const FFVConditionContext& Context, FFVConditionScore& OutScore) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition")
	FText GetDescription() const;

	virtual UWorld* GetWorld() const override;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;
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

	/** Sum of the scored conditions (All, so more requirements rank higher) or the best one (Any); false when none is scored. */
	bool GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const;
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
	virtual bool GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const override { return Set.GetScore(Context, OutScore); }
};

/** Holds a UFVCondition (C++ or AngelScript) in a condition set until sets hold them directly. */
USTRUCT(BlueprintType, meta = (DisplayName = "Object"))
struct FVCORERUNTIME_API FFVCondition_Object : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TObjectPtr<UFVCondition> Condition;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
	virtual bool GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const override;
};
