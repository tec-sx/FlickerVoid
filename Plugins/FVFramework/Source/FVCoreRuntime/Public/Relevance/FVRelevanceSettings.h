#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVRelevanceSettings.generated.h"

UENUM(BlueprintType, meta = (ScriptName = "RelevanceBand"))
enum class EFVRelevanceBand : uint8
{
	Near UMETA(ToolTip = "Within the group's Near Distance."),
	Mid UMETA(ToolTip = "Between the group's Near and Mid Distance."),
	Far UMETA(ToolTip = "Beyond the group's Mid Distance, or not scored yet.")
};

UENUM(BlueprintType, meta = (ScriptName = "RelevanceOrigin"))
enum class EFVRelevanceOrigin : uint8
{
	Pawn UMETA(ToolTip = "Distances are measured from the local player's pawn, falling back to the camera when there is no pawn."),
	Camera UMETA(ToolTip = "Distances are measured from the local player's camera.")
};

/** Band thresholds for one relevance group. */
USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVRelevanceGroup
{
	GENERATED_BODY()

	EFVRelevanceBand ComputeBand(float Distance, EFVRelevanceBand CurrentBand) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relevance", meta = (Categories = "Relevance", ToolTip = "Group these thresholds apply to, as set on relevance components."))
	FGameplayTag Group;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relevance", meta = (ClampMin = 0, Units = "cm", ToolTip = "Objects closer than this are in the Near band."))
	float NearDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relevance", meta = (ClampMin = 0, Units = "cm", ToolTip = "Objects closer than this (and beyond Near Distance) are in the Mid band; the rest are Far."))
	float MidDistance = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relevance", meta = (ClampMin = 0, Units = "cm", ToolTip = "Extra distance an object must move past a threshold before it leaves a band, so it doesn't flicker at the edge."))
	float Hysteresis = 50.f;
};

/** Distance bands for the relevance system. Groups classify objects (interactables, scan targets, vehicles) and each has its own thresholds. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Relevance"))
class FVCORERUNTIME_API UFVRelevanceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UFVRelevanceSettings& Get() { return *GetDefault<UFVRelevanceSettings>(); }

	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	const FFVRelevanceGroup* FindGroup(const FGameplayTag& Group) const;

	UPROPERTY(Config, EditAnywhere, Category = "Relevance", meta = (ToolTip = "Where distances are measured from."))
	EFVRelevanceOrigin Origin = EFVRelevanceOrigin::Pawn;

	UPROPERTY(Config, EditAnywhere, Category = "Relevance", meta = (ClampMin = 0, Units = "s", ToolTip = "Real seconds between relevance updates. 0 updates every frame."))
	float UpdateInterval = 0.1f;

	UPROPERTY(Config, EditAnywhere, Category = "Relevance", meta = (TitleProperty = "Group", ToolTip = "Band thresholds per group."))
	TArray<FFVRelevanceGroup> Groups;
};
