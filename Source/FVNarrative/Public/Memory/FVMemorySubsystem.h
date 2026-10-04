#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"

#include "FVMemorySubsystem.generated.h"

class UFVMemoryFragment;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnMemoryDiscovered, const UFVMemoryFragment*, Memory, AActor*, Discoverer);

/**
 * UFVMemorySubsystem
 *
 * Tracks which memory fragments the protagonist has recovered and applies their impact:
 * fragment count and sanity on the discoverer's attributes, identity recovery, and world state tags.
 * Quest unlocks and presentation are left to OnMemoryDiscovered listeners.
 */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVMemorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UFVMemorySubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Memory")
	bool HasDiscovered(const UFVMemoryFragment* Memory) const;

	/** True when the memory is not yet recovered and all its prerequisite memories are. */
	UFUNCTION(BlueprintPure, Category = "Memory")
	bool CanDiscover(const UFVMemoryFragment* Memory) const;

	/** Recovers the memory and applies its impact. Returns false if already known or prerequisites are missing. */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	bool DiscoverMemory(const UFVMemoryFragment* Memory, AActor* Discoverer);

	UFUNCTION(BlueprintPure, Category = "Memory")
	TArray<FPrimaryAssetId> GetDiscoveredMemories() const { return DiscoveredMemories.Array(); }

	/** Sum of IdentityContribution of every recovered memory, 0 to 1. */
	UFUNCTION(BlueprintPure, Category = "Memory")
	float GetIdentityRecovery() const { return IdentityRecovery; }

	UPROPERTY(BlueprintAssignable, Category = "Memory")
	FFVOnMemoryDiscovered OnMemoryDiscovered;

private:
	void ApplyAttributeImpact(const UFVMemoryFragment& Memory, AActor* Discoverer) const;

	UPROPERTY(SaveGame)
	TSet<FPrimaryAssetId> DiscoveredMemories;

	UPROPERTY(SaveGame)
	float IdentityRecovery = 0.f;
};
