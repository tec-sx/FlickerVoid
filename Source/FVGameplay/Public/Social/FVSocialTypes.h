#pragma once

#include "CoreMinimal.h"
#include "Data/FVCharacterDefinition.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVSocialTypes.generated.h"

class UFVFactionDefinition;
class UFVScaleDefinition;
class UFVTitleDefinition;

UENUM(BlueprintType, meta = (ScriptName = "Attitude"))
enum class EFVAttitude : uint8
{
	Hostile UMETA(ToolTip = "Attacks or reports on sight."),
	Unfriendly UMETA(ToolTip = "Refuses help and raises prices."),
	Neutral UMETA(ToolTip = "No opinion either way."),
	Friendly UMETA(ToolTip = "Helps and gives better prices."),
	Allied UMETA(ToolTip = "Treats the subject as one of their own.")
};

UENUM(BlueprintType, meta = (ScriptName = "ContextActor"))
enum class EFVContextActor : uint8
{
	Instigator UMETA(ToolTip = "The context's instigator, usually the player."),
	Target UMETA(ToolTip = "The context's target, usually the NPC.")
};

/** Social data of a character: the faction they belong to and the scale holding the player's relationship with them. */
USTRUCT(BlueprintType, meta = (DisplayName = "Social"))
struct FLICKERVOIDGAMEPLAY_API FFVCharacterFragment_Social : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social", meta = (ToolTip = "Scale holding the player's relationship with this character; its fact's Facts entry gives the range and starting value."))
	TObjectPtr<UFVScaleDefinition> Relationship;
};

/** A kind of gift a character reacts to. */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVGiftPreference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gifts", meta = (ToolTip = "Matched against the gift's definition Tags and Category; a parent tag matches every child."))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gifts", meta = (ToolTip = "Relationship change when a gift matches; negative for gifts the character dislikes."))
	int32 RelationshipChange = 1;
};

/** How a character takes gifts: matching preferences add up, a gift matching none gets the default change. */
USTRUCT(BlueprintType, meta = (DisplayName = "Gifts"))
struct FLICKERVOIDGAMEPLAY_API FFVCharacterFragment_Gifts : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gifts", meta = (TitleProperty = "Tag", MaxEntries = 3, ToolTip = "Gifts this character likes or dislikes."))
	TArray<FFVGiftPreference> Preferences;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gifts", meta = (ToolTip = "Relationship change for a gift no preference matches."))
	int32 DefaultRelationshipChange = 0;
};

/** Game-wide social setup: the fame and notoriety scales, the titles, factions and characters the game tracks, and disguises. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Social"))
class FLICKERVOIDGAMEPLAY_API UFVSocialSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVSocialSettings& Get() { return *GetDefault<UFVSocialSettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	UPROPERTY(Config, EditAnywhere, Category = "Fame", meta = (ToolTip = "How widely the player is talked about, whatever factions think of them. Faction standing scales feed it through their links."))
	TSoftObjectPtr<UFVScaleDefinition> Fame;

	UPROPERTY(Config, EditAnywhere, Category = "Fame", meta = (ToolTip = "How wanted the player is overall. Faction standing scales feed it through their links."))
	TSoftObjectPtr<UFVScaleDefinition> Notoriety;

	UPROPERTY(Config, EditAnywhere, Category = "Fame", meta = (ToolTip = "Titles the player can earn; they are checked whenever a fact changes."))
	TArray<TSoftObjectPtr<UFVTitleDefinition>> Titles;

	UPROPERTY(Config, EditAnywhere, Category = "Factions", meta = (ToolTip = "Factions the menus, disguises and debug commands know about."))
	TArray<TSoftObjectPtr<UFVFactionDefinition>> Factions;

	UPROPERTY(Config, EditAnywhere, Category = "Relationships", meta = (ToolTip = "Characters the menus list once their relationship scale has been set."))
	TArray<TSoftObjectPtr<UFVCharacterDefinition>> Relationships;

	UPROPERTY(Config, EditAnywhere, Category = "Appearance", meta = (Categories = "Disguise", ToolTip = "Wearing anything tagged under this root hides who the player is."))
	FGameplayTag DisguiseRoot;

	UPROPERTY(Config, EditAnywhere, Category = "Appearance", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "Fame and notoriety others read while the player is disguised, as a fraction of the real value."))
	float DisguiseRecognition = 0.25f;
};
