#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVReputation.generated.h"

USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVReputationTier
{
GENERATED_BODY()

/** Standing at or above which this tier applies. */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
int32 MinStanding = 0;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
FGameplayTag Tier;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation")
FText Label;
};

/** A faction. Id is the fact tag holding current standing (e.g. Reputation.Faction.Police). */
UCLASS(BlueprintType)
class FVSTORYSYSTEM_API UFVFactionDefinition : public UFVDefinition
{
GENERATED_BODY()

public:
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
int32 MinStanding = -100;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
int32 MaxStanding = 100;

/** Sorted ascending by MinStanding. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
TArray<FFVReputationTier> Tiers;

/** Change applied to other factions, as a fraction of the change to this one. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reputation")
TMap<TObjectPtr<UFVFactionDefinition>, float> Relations;

const FFVReputationTier* FindTier(int32 Standing) const;
};

UCLASS()
class FVSTORYSYSTEM_API UFVReputationStatics : public UBlueprintFunctionLibrary
{
GENERATED_BODY()

public:
UFUNCTION(BlueprintPure, Category = "Reputation", meta = (WorldContext = "WorldContext"))
static int32 GetStanding(const UObject* WorldContext, const UFVFactionDefinition* Faction);

UFUNCTION(BlueprintPure, Category = "Reputation", meta = (WorldContext = "WorldContext"))
static FGameplayTag GetTier(const UObject* WorldContext, const UFVFactionDefinition* Faction);

/** Clamped change; propagates to related factions once (no chaining). */
UFUNCTION(BlueprintCallable, Category = "Reputation", meta = (WorldContext = "WorldContext"))
static void ModifyStanding(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta);

private:
static void ApplyDelta(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta);
};

USTRUCT(BlueprintType, meta = (DisplayName = "Reputation Standing"))
struct FVSTORYSYSTEM_API FFVCondition_Standing : public FFVConditionBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Condition")
TObjectPtr<UFVFactionDefinition> Faction;

UPROPERTY(EditAnywhere, Category = "Condition")
int32 MinStanding = 0;

virtual FText GetDescription() const override;

protected:
virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Reputation"))
struct FVSTORYSYSTEM_API FFVEffect_ModifyStanding : public FFVEffectBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Effect")
TObjectPtr<UFVFactionDefinition> Faction;

UPROPERTY(EditAnywhere, Category = "Effect")
int32 Delta = 0;

virtual void Apply(const FFVConditionContext& Context) const override;
virtual FText GetDescription() const override;
};