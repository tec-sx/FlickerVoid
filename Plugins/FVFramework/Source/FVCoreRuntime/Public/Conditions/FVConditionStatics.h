#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "FVConditionStatics.generated.h"

UCLASS()
class FVCORERUNTIME_API UFVConditionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Condition", meta = (DefaultToSelf = "Instigator"))
	static bool EvaluateConditionSet(const FFVConditionSet& Set, AActor* Instigator, AActor* Target);

	UFUNCTION(BlueprintPure, Category = "FV|Condition")
	static FText DescribeConditionSet(const FFVConditionSet& Set);

	UFUNCTION(BlueprintCallable, Category = "FV|Effect", meta = (DefaultToSelf = "Instigator"))
	static void ApplyEffects(const FFVEffectList& Effects, AActor* Instigator, AActor* Target);

	static FFVConditionContext MakeContext(AActor* Instigator, AActor* Target);
};
