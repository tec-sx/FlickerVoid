#pragma once

#include "Engine/DataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"

#include "FVAbilitySet.generated.h"

class UFVInteractAbility;
class UAttributeSet;
class UGameplayEffect;
class UFVAbilitySystemComponent;
class UFVGameplayAbility;
class UObject;

USTRUCT(BlueprintType)
struct FFVAbilitySet_GameplayAbility
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFVGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FFVAbilitySet_InteractAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFVInteractAbility> Ability;
	
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;
};


USTRUCT(BlueprintType)
struct FFVAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 * FFVAbilitySet_AttributeSet
 *
 *	Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FFVAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

};

/**
 * FFVAbilitySet_GrantedHandles
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FFVAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeFromAbilitySystem(UFVAbilitySystemComponent* FVASC);

protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void PassToAbilitySystem(UFVAbilitySystemComponent* FVASC, FFVAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability))
	TArray<FFVAbilitySet_GameplayAbility> GrantedGameplayAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction Abilities", meta = (TitleProperty = Ability))
	TArray<FFVAbilitySet_InteractAbility> GrantedInteractionAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect))
	TArray<FFVAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta = (TitleProperty = AttributeSet))
	TArray<FFVAbilitySet_AttributeSet> GrantedAttributes;
};
