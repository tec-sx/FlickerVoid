#pragma once

#include "CoreMinimal.h"
#include "Data/FVDisplayInfo.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FVAIConfigData.generated.h"

class UAISenseConfig;
class UAISense;
class UFVFactionDefinition;
class UStateTree;

/** Data for one NPC type: identity, faction, behavior and perception. */
UCLASS()
class FLICKERVOIDAI_API UFVAIConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Used to build Fact.NPC.<NpcId>.* facts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName NpcId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FFVDisplayInfo Display;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	TObjectPtr<UFVFactionDefinition> Faction;

	/** Used when the character has no StateTree set on the instance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	TObjectPtr<UStateTree> DefaultStateTree;

	/** Added to the character's owned tags on BeginPlay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
	FGameplayTagContainer StartingTags;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	TSubclassOf<UAISense> DominantSense;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Perception")
	TArray<TObjectPtr<UAISenseConfig>> SensesConfig;
};