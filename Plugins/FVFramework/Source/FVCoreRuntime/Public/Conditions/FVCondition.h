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

	/** A condition that can't be evaluated (see CanEvaluate) fails, whatever Invert says. */
	bool Evaluate(const FFVConditionContext& Context) const { return CanEvaluate(Context) && EvaluateImpl(Context) != bInvert; }
	virtual FText GetDescription() const { return FText::GetEmpty(); }

	/** Why the condition can't work as set up (a missing reference or tag); empty when it's fine. Data validation reports it. */
	virtual FText GetSetupError() const { return FText::GetEmpty(); }

	/** Scores an at-least condition (fame >= 80) to rank what it unlocks; false for other conditions and inverted ones. */
	bool GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const { return !bInvert && CanEvaluate(Context) && GetScoreImpl(Context, OutScore); }

protected:
	/** False when the condition can't be evaluated here: a setup error, or a context actor it needs is missing. */
	virtual bool CanEvaluate(const FFVConditionContext& Context) const { return GetSetupError().IsEmpty(); }
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
	UFUNCTION(BlueprintCallable, Category = "FV|Condition", meta = (ToolTip = "Evaluate's result, flipped when Invert is set. False whatever Invert says when Can Evaluate is false."))
	bool Check(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintCallable, Category = "FV|Condition", meta = (ToolTip = "Ranks what an at-least condition unlocks; false when the condition isn't scored or is inverted."))
	bool GetScore(const FFVConditionContext& Context, FFVConditionScore& OutScore) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition", meta = (ToolTip = "Whether the condition holds, before Invert. Override it; the default fails and warns. Read the world through Context.WorldContext: the condition lives in an asset, which has no world."))
	bool Evaluate(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition", meta = (ToolTip = "Override to return false when the condition can't be evaluated here (a context actor it needs is missing); it then fails whatever Invert says. Default: true when there is no setup error."))
	bool CanEvaluate(const FFVConditionContext& Context) const;

	UFUNCTION(BlueprintNativeEvent, Category = "FV|Condition", meta = (ToolTip = "Override to say why the condition can't work as set up (a missing reference or tag); empty when it's fine. Data validation reports it, and the condition fails while it's set."))
	FText GetSetupError() const;

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
	/** An entry with no type picked counts as failed. */
	bool Evaluate(const FFVConditionContext& Context) const;
	FText GetDescription() const;

	/** Every entry's setup error, numbered; empty when the set is fine. */
	FText GetSetupError() const;

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
	virtual FText GetSetupError() const override { return Set.GetSetupError(); }

protected:
	virtual bool CanEvaluate(const FFVConditionContext& Context) const override { return true; }
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
	virtual FText GetSetupError() const override;

protected:
	virtual bool CanEvaluate(const FFVConditionContext& Context) const override;
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
	virtual bool GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const override;
};
