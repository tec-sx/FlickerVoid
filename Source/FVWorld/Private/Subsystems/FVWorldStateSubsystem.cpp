// Fill out your copyright notice in the Description page of Project Settings.
#include "Subsystems/FVWorldStateSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVWorldStateSubsystem)

void UFVWorldStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentGameTimeHours = 0.0f;
}

void UFVWorldStateSubsystem::Deinitialize()
{
	WorldStateTags.Reset();
	NPCRelationships.Empty();
	DiscoveredLocations.Empty();

	Super::Deinitialize();
}

void UFVWorldStateSubsystem::AddWorldStateTag(FGameplayTag Tag)
{
	if (Tag.IsValid() && !WorldStateTags.HasTagExact(Tag))
	{
		WorldStateTags.AddTag(Tag);
		OnWorldStateTagChanged.Broadcast(Tag, true);
	}
}

void UFVWorldStateSubsystem::RemoveWorldStateTag(FGameplayTag Tag)
{
	if (Tag.IsValid() && WorldStateTags.HasTagExact(Tag))
	{
		WorldStateTags.RemoveTag(Tag);
		OnWorldStateTagChanged.Broadcast(Tag, false);
	}
}

bool UFVWorldStateSubsystem::HasWorldStateTag(FGameplayTag Tag) const
{
	return WorldStateTags.HasTagExact(Tag);
}

bool UFVWorldStateSubsystem::HasAnyWorldStateTags(const FGameplayTagContainer& Tags) const
{
	return WorldStateTags.HasAny(Tags);
}

bool UFVWorldStateSubsystem::HasAllWorldStateTags(const FGameplayTagContainer& Tags) const
{
	return WorldStateTags.HasAll(Tags);
}

void UFVWorldStateSubsystem::SetNPCRelationship(FName NPCId, float Value)
{
	const float ClampedValue = FMath::Clamp(Value, -1.0f, 1.0f);
	const float OldValue = GetNPCRelationship(NPCId);
	
	NPCRelationships.Add(NPCId, ClampedValue);
	
	if (!FMath::IsNearlyEqual(OldValue, ClampedValue))
	{
		OnNPCRelationshipChanged.Broadcast(NPCId, OldValue, ClampedValue);
	}
}

void UFVWorldStateSubsystem::ModifyNPCRelationship(FName NPCId, float Delta)
{
	const float CurrentValue = GetNPCRelationship(NPCId);
	SetNPCRelationship(NPCId, CurrentValue + Delta);
}

float UFVWorldStateSubsystem::GetNPCRelationship(FName NPCId) const
{
	const float* Found = NPCRelationships.Find(NPCId);
	return Found ? *Found : 0.0f;
}

void UFVWorldStateSubsystem::DiscoverLocation(FName LocationId)
{
	if (LocationId != NAME_None)
	{
		DiscoveredLocations.Add(LocationId);
	}
}

bool UFVWorldStateSubsystem::HasDiscoveredLocation(FName LocationId) const
{
	return DiscoveredLocations.Contains(LocationId);
}

TArray<FName> UFVWorldStateSubsystem::GetDiscoveredLocations() const
{
	return DiscoveredLocations.Array();
}

void UFVWorldStateSubsystem::AdvanceGameTime(float Hours)
{
	if (Hours > 0.0f)
	{
		CurrentGameTimeHours += Hours;
	}
}

TArray<uint8> UFVWorldStateSubsystem::SerializeWorldState() const
{
	// TODO: Implement proper serialization using FMemoryWriter
	TArray<uint8> Data;
	return Data;
}

void UFVWorldStateSubsystem::DeserializeWorldState(const TArray<uint8>& Data)
{
	// TODO: Implement proper deserialization using FMemoryReader
}