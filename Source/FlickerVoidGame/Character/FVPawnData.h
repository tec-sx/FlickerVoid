#pragma once

#include "Engine/DataAsset.h"
#include "FVPawnData.generated.h"

#define UE_API FLICKERVOID_API

class APawn;
class UFVAbilitySet;
class UFVAbilityTagRelationshipMap;
class UFVInputConfig;
class UFVCharacterDataAsset;
class UObject;

/**
 * UFVPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "FlickerVoid Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class UFVPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UE_API UFVPawnData();
	
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FlickerVoid|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FlickerVoid|Abilities")
	TArray<TObjectPtr<UFVAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FlickerVoid|Abilities")
	TObjectPtr<UFVAbilityTagRelationshipMap> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FVoid|Input")
	TObjectPtr<UFVInputConfig> InputConfig;

	// Character data asset defining attributes, personality, and initial effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FVoid|Character")
	TObjectPtr<UFVCharacterDataAsset> CharacterData;
};

#undef UE_API
