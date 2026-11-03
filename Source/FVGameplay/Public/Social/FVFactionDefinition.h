#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "Social/FVSocialTypes.h"
#include "FVFactionDefinition.generated.h"

class UFVScaleDefinition;

/**
 * A faction. Id identifies it (stolen goods remember it); the player's standing and notoriety with it are scales, and
 * the standing scale's links feed fame, global notoriety and related factions.
 */
UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVFactionDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Attitude members take toward someone at this standing, from the standing band reached. */
	EFVAttitude GetStandingAttitude(int32 Value) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Standing", meta = (ToolTip = "The player's standing with the faction. Its bands name the tiers; its links pass changes on to fame, notoriety and related factions."))
	TObjectPtr<UFVScaleDefinition> Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Standing", meta = (ForceInlineRow, ToolTip = "Attitude per standing band tag. Below every band, or for a band not listed, members are Neutral."))
	TMap<FGameplayTag, EFVAttitude> BandAttitudes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notoriety", meta = (ToolTip = "How wanted the player is by this faction."))
	TObjectPtr<UFVScaleDefinition> Notoriety;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notoriety", meta = (ClampMin = 0, ToolTip = "At or above this notoriety, as the faction reads it, members are hostile regardless of standing."))
	int32 HostileNotoriety = 75;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Disguise", meta = (Categories = "Disguise", ToolTip = "Tag (granted by outfits) that makes someone pass as a member."))
	FGameplayTag DisguiseTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Disguise", meta = (ClampMin = 0, ToolTip = "Disguises stop working at or above this notoriety, faction or global."))
	int32 DisguiseNotorietyLimit = 50;

protected:
	virtual bool RequiresId() const override { return true; }
};
