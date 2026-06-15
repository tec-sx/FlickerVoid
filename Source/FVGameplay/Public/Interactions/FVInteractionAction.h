#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVInteractionAction.generated.h"

struct FFVInteractionActionInfo;
class UStateTree;

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionAction : public UDataAsset
{
	GENERATED_BODY()

public:
	//~=========================================================================
	// Identity
	//~=========================================================================
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag.Ability"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsSimple;
	
	//~=========================================================================
	// Execution
	//~=========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "!bIsSimple"))
	TObjectPtr<UStateTree> ActionStateTree;
	
	UFUNCTION(BlueprintCallable)
	bool CheckRequirements(FGameplayTagContainer& InstigatorTags) const;
	
	UFUNCTION(BlueprintCallable)
	FFVInteractionActionInfo CreateActionUIInfo(FGameplayTagContainer& InstigatorTags) const;
	
	UFUNCTION(BlueprintCallable)
	FGameplayTagContainer& GetGrantedTags() { return GrantedTags; }
	
private:
	
	//~=========================================================================
	// Requirements And Consequences
	//~=========================================================================
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer RequiredTags;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer BlockedByTags;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer GrantedTags;
};
