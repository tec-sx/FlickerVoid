#pragma once

#include "CoreMinimal.h"
#include "Data/FVCharacterDefinition.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVSocialTypes.generated.h"

class UFVFactionDefinition;

UENUM(BlueprintType)
enum class EFVAttitude : uint8
{
	Hostile,
	Unfriendly,
	Neutral,
	Friendly,
	Allied
};

UENUM(BlueprintType)
enum class EFVContextActor : uint8
{
	Instigator,
	Target
};

/** Social data of a character: faction membership and the fact holding the player's relationship with them. */
USTRUCT(BlueprintType, meta = (DisplayName = "Social"))
struct FVSOCIALSYSTEM_API FFVCharacterFragment_Social : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social")
	TObjectPtr<UFVFactionDefinition> Faction;

	/** Fact holding the player's relationship with this character. Range and default come from Facts settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social", meta = (Categories = "Fact"))
	FGameplayTag RelationshipFact;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Social"))
class FVSOCIALSYSTEM_API UFVSocialSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	/** Fact holding the player's personal reputation, independent of factions. */
	UPROPERTY(Config, EditAnywhere, Category = "Reputation", meta = (Categories = "Fact"))
	FGameplayTag PersonalReputationFact;

	/** Factions whose notoriety decays over game time. */
	UPROPERTY(Config, EditAnywhere, Category = "Notoriety")
	TArray<TSoftObjectPtr<UFVFactionDefinition>> Factions;

	/** Notoriety removed per in-game hour. */
	UPROPERTY(Config, EditAnywhere, Category = "Notoriety", meta = (ClampMin = 0))
	int32 NotorietyDecayPerHour = 5;
};
