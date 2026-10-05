#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "FVSocialTypes.h"
#include "FVFactionDefinition.generated.h"

USTRUCT(BlueprintType)
struct FVSOCIALSYSTEM_API FFVReputationTier
{
	GENERATED_BODY()

	/** Standing at or above which this tier applies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 MinStanding = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	FGameplayTag Tier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
	EFVAttitude Attitude = EFVAttitude::Neutral;
};

/** A faction. Id is the fact holding the player's standing with it (e.g. Fact.Faction.Police.Standing). */
UCLASS(BlueprintType)
class FVSOCIALSYSTEM_API UFVFactionDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
	int32 MinStanding = -100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
	int32 MaxStanding = 100;

	/** Sorted ascending by MinStanding. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation", meta = (TitleProperty = "Label"))
	TArray<FFVReputationTier> Tiers;

	/** Change applied to other factions, as a fraction of the change to this one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
	TMap<TObjectPtr<UFVFactionDefinition>, float> Relations;

	/** How much standing gained here feeds the player's fame. Negative for factions nobody admires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
	float FameContribution = 1.f;

	/** How much standing gained here feeds global notoriety: raise it for criminal factions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notoriety")
	float NotorietyContribution = 0.f;

	/** Fact holding how wanted the player is by this faction (0-100). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notoriety", meta = (Categories = "Fact"))
	FGameplayTag NotorietyFact;

	/** At or above this notoriety members are hostile regardless of standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notoriety", meta = (ClampMin = 0, ClampMax = 100))
	int32 HostileNotoriety = 75;

	/** Tag (granted by outfits) that makes someone pass as a member. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Disguise", meta = (Categories = "Disguise"))
	FGameplayTag DisguiseTag;

	/** Disguises stop working at or above the notoriety this faction reads. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Disguise", meta = (ClampMin = 0, ClampMax = 100))
	int32 DisguiseNotorietyLimit = 50;

	const FFVReputationTier* FindTier(int32 Standing) const;
};
