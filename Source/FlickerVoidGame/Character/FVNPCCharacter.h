#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "FVNPCCharacter.generated.h"

class UFVInteractionTargetComponent;
class UAbilitySystemComponent;

//~=============================================================================
// Lightweight NPC — no custom movement stack, just a character with an
// interactable component and a gameplay tag container for traits/state.
//
// For quest-giving, dialogue, and trading add further components
// (UFVQuestGiverComponent, UFVDialogueComponent) from FVNarrative module.
//~=============================================================================

UCLASS(Config = Game, Blueprintable)
class FLICKERVOID_API AFVNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFVNPCCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~=========================================================================
	// Identity
	//~=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FGameplayTagContainer CharacterTags;

	//~=========================================================================
	// Components
	//~=========================================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC")
	TObjectPtr<UFVInteractionTargetComponent> InteractableComponent;

	//~=========================================================================
	// ACharacter Interface
	//~=========================================================================

	virtual void BeginPlay() override;

	//~=========================================================================
	// Tag Helpers
	//~=========================================================================

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool HasNPCTag(const FGameplayTag& Tag) const { return CharacterTags.HasTag(Tag); }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void AddNPCTag(const FGameplayTag& Tag) { CharacterTags.AddTag(Tag); }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void RemoveNPCTag(const FGameplayTag& Tag) { CharacterTags.RemoveTag(Tag); }
};
