#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Conditions/FVCondition.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVCheck.generated.h"

class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EFVCheckRoll : uint8
{
None,
D6x2,
D10,
D20
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVCheckModifier
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
FText Label;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
FGameplayTag RequiredTag;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Check")
int32 Bonus = 0;
};

UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVCheckDefinition : public UPrimaryDataAsset
{
GENERATED_BODY()

public:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
FText Name;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
FGameplayAttribute Attribute;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
EFVCheckRoll Roll = EFVCheckRoll::D6x2;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Check")
TArray<FFVCheckModifier> Modifiers;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVCheckLine
{
GENERATED_BODY()

UPROPERTY(BlueprintReadOnly, Category = "Check")
FText Label;

UPROPERTY(BlueprintReadOnly, Category = "Check")
int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVCheckResult
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
class FLICKERVOIDGAMEPLAY_API UFVCheckStatics : public UBlueprintFunctionLibrary
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintCallable, Category = "Check")
static FFVCheckResult RollCheck(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty);

UFUNCTION(BlueprintPure, Category = "Check")
static FFVCheckResult PreviewCheck(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty);

UFUNCTION(BlueprintPure, Category = "Check")
static float SuccessChance(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty);

static UAbilitySystemComponent* FindASC(const AActor* Actor);

private:
static FFVCheckResult Build(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty, bool bRoll);
static int32 Roll(EFVCheckRoll InRoll);
};

USTRUCT(BlueprintType, meta = (DisplayName = "Skill Check (Passive)"))
struct FLICKERVOIDGAMEPLAY_API FFVCondition_PassiveCheck : public FFVConditionBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Condition")
TObjectPtr<UFVCheckDefinition> Check;

UPROPERTY(EditAnywhere, Category = "Condition")
int32 Difficulty = 10;

virtual FText GetDescription() const override;

protected:
virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Has Gameplay Tag"))
struct FLICKERVOIDGAMEPLAY_API FFVCondition_HasTag : public FFVConditionBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Condition")
FGameplayTag Tag;

UPROPERTY(EditAnywhere, Category = "Condition")
bool bCheckTarget = false;

virtual FText GetDescription() const override;

protected:
virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};