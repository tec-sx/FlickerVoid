// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"

#include "FVWorldStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldStateTagChanged, FGameplayTag, Tag, bool, bAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNPCRelationshipChanged, FName, NPCId, float, OldValue, float, NewValue);

/**
 * UFVWorldStateSubsystem
 * 
 * Tracks persistent world state that affects narrative progression.
 * - World state tags (story flags, unlocks, consequences)
 * - NPC relationships
 * - Discovered locations
 * - Time tracking
 */
UCLASS()
class FLICKERVOIDWORLD_API UFVWorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// WORLD STATE TAGS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "WorldState")
	void AddWorldStateTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "WorldState")
	void RemoveWorldStateTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState")
	bool HasWorldStateTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState")
	bool HasAnyWorldStateTags(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState")
	bool HasAllWorldStateTags(const FGameplayTagContainer& Tags) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState")
	const FGameplayTagContainer& GetAllWorldStateTags() const { return WorldStateTags; }

	// ========================================================================
	// NPC RELATIONSHIPS
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "WorldState|NPC")
	void SetNPCRelationship(FName NPCId, float Value);

	UFUNCTION(BlueprintCallable, Category = "WorldState|NPC")
	void ModifyNPCRelationship(FName NPCId, float Delta);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|NPC")
	float GetNPCRelationship(FName NPCId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|NPC")
	TMap<FName, float> GetAllNPCRelationships() const { return NPCRelationships; }

	// ========================================================================
	// LOCATION TRACKING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "WorldState|Location")
	void DiscoverLocation(FName LocationId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|Location")
	bool HasDiscoveredLocation(FName LocationId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|Location")
	TArray<FName> GetDiscoveredLocations() const;

	// ========================================================================
	// TIME TRACKING
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "WorldState|Time")
	void AdvanceGameTime(float Hours);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|Time")
	float GetCurrentGameTime() const { return CurrentGameTimeHours; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|Time")
	int32 GetCurrentDay() const { return FMath::FloorToInt(CurrentGameTimeHours / 24.0f); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldState|Time")
	float GetTimeOfDay() const { return FMath::Fmod(CurrentGameTimeHours, 24.0f); }

	// ========================================================================
	// EVENTS
	// ========================================================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWorldStateTagChanged OnWorldStateTagChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNPCRelationshipChanged OnNPCRelationshipChanged;

	// ========================================================================
	// SERIALIZATION
	// ========================================================================

	UFUNCTION(BlueprintCallable, Category = "WorldState|Save")
	TArray<uint8> SerializeWorldState() const;

	UFUNCTION(BlueprintCallable, Category = "WorldState|Save")
	void DeserializeWorldState(const TArray<uint8>& Data);

private:
	UPROPERTY()
	FGameplayTagContainer WorldStateTags;

	UPROPERTY()
	TMap<FName, float> NPCRelationships;

	UPROPERTY()
	TSet<FName> DiscoveredLocations;

	UPROPERTY()
	float CurrentGameTimeHours = 0.0f;
};