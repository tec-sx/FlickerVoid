#pragma once

#include "CoreMinimal.h"
#include "FVSocialTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVSocialStatics.generated.h"

class UFVCharacterDefinition;
class UFVFactionDefinition;

/** Reads and writes social state. All values live in facts, so they save and work in conditions. */
UCLASS()
class FVSOCIALSYSTEM_API UFVSocialStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static int32 GetStanding(const UObject* WorldContext, const UFVFactionDefinition* Faction);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static FGameplayTag GetTier(const UObject* WorldContext, const UFVFactionDefinition* Faction);

	/** Clamped change; propagates to related factions once (no chaining). */
	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static void ModifyStanding(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static int32 GetNotoriety(const UObject* WorldContext, const UFVFactionDefinition* Faction);

	/** Clamped to 0-100. */
	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static void ModifyNotoriety(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static int32 GetPersonalReputation(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static void ModifyPersonalReputation(UObject* WorldContext, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static int32 GetRelationship(const UObject* WorldContext, const UFVCharacterDefinition* Character);

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (WorldContext = "WorldContext"))
	static void ModifyRelationship(UObject* WorldContext, const UFVCharacterDefinition* Character, int32 Delta);

	/** Faction of an actor through its identity's Social fragment. */
	UFUNCTION(BlueprintPure, Category = "FV|Social")
	static UFVFactionDefinition* GetActorFaction(const AActor* Actor);

	/** Actor carries the faction's disguise tag and the faction's notoriety is below its disguise limit. */
	UFUNCTION(BlueprintPure, Category = "FV|Social")
	static bool IsDisguisedAs(const AActor* Actor, const UFVFactionDefinition* Faction);

	/**
	 * How members of Faction treat Subject: members and accepted disguises are Allied,
	 * high notoriety is Hostile, otherwise the standing tier decides.
	 */
	UFUNCTION(BlueprintPure, Category = "FV|Social")
	static EFVAttitude GetAttitude(const UFVFactionDefinition* Faction, const AActor* Subject);

	static const FFVCharacterFragment_Social* FindSocialFragment(const AActor* Actor);

private:
	static void AddClamped(UObject* WorldContext, FGameplayTag Fact, int32 Delta, int32 Min, int32 Max);
};
