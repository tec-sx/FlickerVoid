#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"

#include "FVMemorySubsystem.generated.h"

class UFVMemoryFragment;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnMemoryDiscovered, const UFVMemoryFragment*, Memory, AActor*, Discoverer);

/** Tracks recovered memory fragments and applies their impact. */
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVMemorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UFVMemorySubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintPure, Category = "Memory")
	bool HasDiscovered(const UFVMemoryFragment* Memory) const;

	UFUNCTION(BlueprintPure, Category = "Memory")
	bool CanDiscover(const UFVMemoryFragment* Memory) const;

	UFUNCTION(BlueprintCallable, Category = "Memory")
	bool DiscoverMemory(const UFVMemoryFragment* Memory, AActor* Discoverer);

	UFUNCTION(BlueprintPure, Category = "Memory")
	TArray<FPrimaryAssetId> GetDiscoveredMemories() const { return DiscoveredMemories.Array(); }

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
