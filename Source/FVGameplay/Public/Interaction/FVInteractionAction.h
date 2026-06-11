#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interaction/FVInteractionTypes.h"
#include "FVInteractionAction.generated.h"

class UFVInteractionRequirement;
class UStateTree;

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class FLICKERVOIDGAMEPLAY_API UFVInteractionRequirement : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Requirement")
	bool IsMet(AActor* Instigator) const;
	virtual bool IsMet_Implementation(AActor* Instigator) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|Requirement")
	FText GetUnmetReason(AActor* Instigator) const;
	virtual FText GetUnmetReason_Implementation(AActor* Instigator) const { return FText::GetEmpty(); }
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionAction
{
	GENERATED_BODY()

	//~=========================================================================
	// Identity & Display
	//~=========================================================================

	// Semantic identifier (e.g. Interaction.Action.PickUp, Interaction.Action.LockPick)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	// Which input slot triggers this action (e.g. Input.Interact.Primary)
	// The input data-asset is the only thing that knows which physical key this maps to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (Categories = "Input.Interact"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	TSoftObjectPtr<UTexture2D> Icon;

	//~=========================================================================
	// Execution
	//~=========================================================================

	// State Tree asset to run when this action fires. Must be configured with
	// UStateTreeComponentSchema. Add UFVInteractionStateTreeTaskBase subclass tasks
	// to define the execution flow (pickup, dialogue, mini-game, etc.).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action|Execution")
	TObjectPtr<UStateTree> ActionStateTree;

	//~=========================================================================
	// Requirements — all must pass for the action to be available
	//~=========================================================================

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Action|Requirements",
		meta = (EditInline))
	TArray<TObjectPtr<UFVInteractionRequirement>> Requirements;

	//~=========================================================================
	// Consequences on Success
	//~=========================================================================

	// Tags applied to the world state / player on successful completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action|Consequences")
	FGameplayTagContainer GrantedTagsOnSuccess;

	//~=========================================================================
	// Helpers
	//~=========================================================================

	bool CheckRequirements(AActor* Instigator, FText& OutFirstUnmetReason) const;
};
