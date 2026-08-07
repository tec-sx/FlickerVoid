#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FVAIConfigData.generated.h"

class UAISenseConfig;
class UAISense;

UCLASS()
class FLICKERVOIDAI_API UFVAIConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	TSubclassOf<UAISense> DominantSense;
    
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Perception")
	TArray<TObjectPtr<UAISenseConfig>> SensesConfig;
};
