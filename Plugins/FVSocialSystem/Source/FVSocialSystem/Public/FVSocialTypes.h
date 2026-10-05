#pragma once

#include "CoreMinimal.h"
#include "Data/FVCharacterDefinition.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVSocialTypes.generated.h"

class UFVFactionDefinition;
class UFVTitleDefinition;

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

	static const UFVSocialSettings& Get() { return *GetDefault<UFVSocialSettings>(); }

	/** Fact holding how widely the player is talked about, whatever factions think of them. */
	UPROPERTY(Config, EditAnywhere, Category = "Fame", meta = (Categories = "Fact"))
	FGameplayTag FameFact;

	/** Fact holding how wanted the player is overall. Faction standing feeds into it. */
	UPROPERTY(Config, EditAnywhere, Category = "Fame", meta = (Categories = "Fact"))
	FGameplayTag NotorietyFact;

	/** Titles the player can earn; each one watches the facts its conditions read. */
	UPROPERTY(Config, EditAnywhere, Category = "Fame")
	TArray<TSoftObjectPtr<UFVTitleDefinition>> Titles;

	/** Factions registered for debug commands and title evaluation. */
	UPROPERTY(Config, EditAnywhere, Category = "Fame")
	TArray<TSoftObjectPtr<UFVFactionDefinition>> Factions;

	/** Wearing anything tagged under this root hides who the player is. */
	UPROPERTY(Config, EditAnywhere, Category = "Appearance", meta = (Categories = "Disguise"))
	FGameplayTag DisguiseRoot;

	/** Fame and notoriety others read while the player is disguised, as a fraction of the real value. */
	UPROPERTY(Config, EditAnywhere, Category = "Appearance", meta = (ClampMin = 0, ClampMax = 1))
	float DisguiseRecognition = 0.25f;
};
