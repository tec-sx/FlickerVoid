 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"

#include "FVCharacterAttributeComponent.generated.h"

#define UE_API FLICKERVOIDCHARACTER_API

class UFVCharacterAttributeSet;
class UFVCharacterDataAsset;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterStateChanged, FGameplayTag, OldState, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, OldValue, float, NewValue);

/**
 * UFVCharacterAttributeComponent
 * 
 * Bridges data-driven character configuration with GAS.
 * Attach this to any character that needs attributes and personality.
 * Assign a UFVCharacterDataAsset to configure the character.
 */
UCLASS(MinimalAPI,ClassGroup=(Character), meta=(BlueprintSpawnableComponent))
class UFVCharacterAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFVCharacterAttributeComponent();

	// ========================================================================
	// CONFIGURATION
	// ========================================================================

	/** The data asset that defines this character's attributes and personality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TObjectPtr<const UFVCharacterDataAsset> CharacterData;

	// ========================================================================
	// RUNTIME STATE
	// ========================================================================

	/** Current character state tag (e.g., Character_State_Idle from FVCoreTags) */
	UPROPERTY(BlueprintReadOnly, Category = "State", meta = (Categories = "Character.State"))
	FGameplayTag CurrentStateTag;

	/** Known characters and their relationship scores */
	UPROPERTY(BlueprintReadWrite, Category = "Relationships")
	TMap<TObjectPtr<AActor>, float> KnownCharacters;

	// ========================================================================
	// EVENTS
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterStateChanged OnCharacterStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributeChanged OnAttributeChanged;

	// ========================================================================
	// INITIALIZATION
	// ========================================================================

	/** Initialize attributes from the assigned CharacterData asset */
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeFromDataAsset();

	/** Initialize attributes from a specific data asset (runtime assignment) */
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeFromDataAssetOverride(const UFVCharacterDataAsset* DataAsset);

	// ========================================================================
	// ATTRIBUTE ACCESS (via GAS)
	// ========================================================================

	/** Get the GAS Ability System Component */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	UE_API UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Get the character attribute set */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	UE_API const UFVCharacterAttributeSet* GetAttributeSet() const;

	/** Get an attribute value by gameplay attribute */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	UE_API float GetAttributeValue(FGameplayAttribute Attribute) const;

	/** Get an attribute value by name (slower, for dynamic access) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	UE_API float GetAttributeValueByName(FName AttributeName) const;
    
	// ========================================================================
	// STATE MANAGEMENT
	// ========================================================================

	/** Change the character's state (applies/removes state tags via GAS) */
	UFUNCTION(BlueprintCallable, Category = "State")
	UE_API void SetCharacterState(FGameplayTag NewStateTag);
    
	/** Check if the character has a specific tag */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
	UE_API bool HasTag(FGameplayTag Tag) const;

	/** Check if the character has any of the specified tags */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
	UE_API bool HasAnyTag(const FGameplayTagContainer& Tags) const;

	// ========================================================================
	// RELATIONSHIPS
	// ========================================================================
 
	// ========================================================================
	// RELATIONSHIPS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "Relationships")
	UE_API void UpdateRelationship(AActor* Character, float Delta);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Relationships")
	UE_API float GetRelationship(AActor* Character) const;

	// ========================================================================
	// DERIVED VALUES (calculated from attributes)
	// ========================================================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Derived")
	UE_API float GetPhysicalPower() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Derived")
	UE_API float GetSocialPower() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Derived")
	UE_API float GetMentalStability() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Derived")
	UE_API float GetCombatReadiness() const;

	// ========================================================================
	// SKILLS (Tag-based)
	// ========================================================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Skills")
	UE_API float GetSkillLevel(FGameplayTag SkillTag) const;

	// ========================================================================
	// EFFECTS AND STATUS
	// ========================================================================
	// Derive composite values
	/** Check if character has a specific effect/status */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Effects")
	UE_API bool HasEffect(FName EffectName) const;

	/** Get modified attribute value (base + modifiers) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	UE_API float GetModifiedAttributeValue(FName AttributeName) const;

	/** Set attribute value directly (for initialization/editor) */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UE_API void SetAttributeValue(FName AttributeName, float Value);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY()
	TMap<FGameplayTag, float> SkillLevels;

	TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;

	void CacheAbilitySystemComponent();
	void ApplyInitialGameplayEffects();
	void ApplyAttributeInitialization();
	void GrantTraitTags();

	void OnAttributeChangedInternal(const FOnAttributeChangeData& Data);
	FDelegateHandle AttributeChangeDelegateHandle;
};

#undef UE_API