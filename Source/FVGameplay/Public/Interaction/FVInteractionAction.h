#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interaction/FVInteractionTypes.h"
#include "FVInteractionAction.generated.h"

class UFVInteractionRequirement;
class UFVInteractionActionHandler;
class UFVInteractionHandlerConfig;

//~=============================================================================
// Handler Config Base — subclass per handler to define designer-editable params
//~=============================================================================

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, CollapseCategories)
class FLICKERVOIDGAMEPLAY_API UFVInteractionHandlerConfig : public UObject
{
	GENERATED_BODY()
};

//~=============================================================================
// Action Entry — one slot on an interactable (max 4 per component)
//~=============================================================================

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action|Handler",
		meta = (MustImplement = "/Script/FlickerVoidGameplay.FVInteractionActionHandler"))
	TSubclassOf<UFVInteractionActionHandler> HandlerClass;

	// Handler-specific configuration (Instanced — each action gets its own instance)
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Action|Handler",
		meta = (EditInline))
	TObjectPtr<UFVInteractionHandlerConfig> HandlerConfig;

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
