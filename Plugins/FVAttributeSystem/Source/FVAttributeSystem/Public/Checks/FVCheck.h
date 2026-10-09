#pragma once

#include "CoreMinimal.h"
#include "Attributes/FVAttributeDefinition.h"
#include "Conditions/FVCondition.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVCheck.generated.h"

UENUM(BlueprintType)
enum class EFVCheckRoll : uint8
{
	None,
	D6x2,
	D10,
	D20
};

/** A bonus that applies when its conditions pass, the same way interaction offers gate themselves. */
USTRUCT(BlueprintType)
struct FVATTRIBUTESYSTEM_API FFVCheckModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	FFVConditionSet Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
	int32 Bonus = 0;
};

UCLASS(BlueprintType)
class FVATTRIBUTESYSTEM_API UFVCheckDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
	TObjectPtr<const UFVAttributeDefinition> Attribute;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
	EFVCheckRoll Roll = EFVCheckRoll::D6x2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check", meta = (TitleProperty = "Label"))
	TArray<FFVCheckModifier> Modifiers;
};

USTRUCT(BlueprintType)
struct FVATTRIBUTESYSTEM_API FFVCheckLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	FText Label;

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FVATTRIBUTESYSTEM_API FFVCheckResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 Total = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	int32 Difficulty = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Check")
	TArray<FFVCheckLine> Breakdown;
};

UCLASS()
class FVATTRIBUTESYSTEM_API UFVCheckLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Check", meta = (DefaultToSelf = "Instigator"))
	static FFVCheckResult RollCheck(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty);

	UFUNCTION(BlueprintPure, Category = "FV|Check", meta = (DefaultToSelf = "Instigator"))
	static FFVCheckResult PreviewCheck(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty);

	UFUNCTION(BlueprintPure, Category = "FV|Check", meta = (DefaultToSelf = "Instigator"))
	static float SuccessChance(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty);

	static FFVCheckResult Build(const UFVCheckDefinition* Check, const FFVConditionContext& Context, int32 Difficulty, bool bRoll);

private:
	static int32 Roll(EFVCheckRoll InRoll);
};

/** Passes when the check would succeed without rolling. */
USTRUCT(BlueprintType, meta = (DisplayName = "Skill Check (Passive)"))
struct FVATTRIBUTESYSTEM_API FFVCondition_PassiveCheck : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<const UFVCheckDefinition> Check;

	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 Difficulty = 10;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};
