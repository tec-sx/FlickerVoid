#pragma once

#include "CoreMinimal.h"
#include "Data/FVCharacterDefinition.h"
#include "FVAbilityFragments.generated.h"

class UFVAbilitySet;

/** Ability sets a character's ability system grants the first time the character is possessed. */
USTRUCT(BlueprintType, meta = (DisplayName = "Abilities"))
struct FLICKERVOIDGAMEPLAY_API FFVCharacterFragment_Abilities : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Abilities", meta = (ToolTip = "Abilities, attribute sets and effects granted to the character."))
	TArray<TObjectPtr<const UFVAbilitySet>> AbilitySets;
};
