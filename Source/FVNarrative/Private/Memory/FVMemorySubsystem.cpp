#include "Memory/FVMemorySubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Attributes/FVProtagonistAttributeSet.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Logging/FVLogCategories.h"
#include "Memory/FVMemoryFragment.h"
#include "Subsystems/FVWorldStateSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMemorySubsystem)

UFVMemorySubsystem* UFVMemorySubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UFVMemorySubsystem>() : nullptr;
}

bool UFVMemorySubsystem::HasDiscovered(const UFVMemoryFragment* Memory) const
{
	return Memory && DiscoveredMemories.Contains(Memory->GetPrimaryAssetId());
}

bool UFVMemorySubsystem::CanDiscover(const UFVMemoryFragment* Memory) const
{
	if (!Memory || HasDiscovered(Memory))
	{
		return false;
	}

	for (const FPrimaryAssetId& Prerequisite : Memory->PrerequisiteMemories)
	{
		if (!DiscoveredMemories.Contains(Prerequisite))
		{
			return false;
		}
	}
	return true;
}

bool UFVMemorySubsystem::DiscoverMemory(const UFVMemoryFragment* Memory, AActor* Discoverer)
{
	if (!CanDiscover(Memory))
	{
		return false;
	}

	DiscoveredMemories.Add(Memory->GetPrimaryAssetId());
	IdentityRecovery = FMath::Clamp(IdentityRecovery + Memory->IdentityContribution, 0.f, 1.f);

	ApplyAttributeImpact(*Memory, Discoverer);

	if (UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
	{
		for (const FGameplayTag& Tag : Memory->GrantedWorldStateTags)
		{
			WorldState->AddWorldStateTag(Tag);
		}
	}

	UE_LOG(LogFVNarrative, Log, TEXT("Memory recovered: %s"), *Memory->GetPrimaryAssetId().ToString());

	OnMemoryDiscovered.Broadcast(Memory, Discoverer);
	return true;
}

void UFVMemorySubsystem::ApplyAttributeImpact(const UFVMemoryFragment& Memory, AActor* Discoverer) const
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Discoverer);
	if (!ASC || !ASC->HasAttributeSetForAttribute(UFVProtagonistAttributeSet::GetMemoryFragmentsFoundAttribute()))
	{
		return;
	}

	ASC->ApplyModToAttribute(UFVProtagonistAttributeSet::GetMemoryFragmentsFoundAttribute(), EGameplayModOp::Additive, 1.f);

	if (!FMath::IsNearlyZero(Memory.SanityImpact))
	{
		ASC->ApplyModToAttribute(UFVProtagonistAttributeSet::GetSanityAttribute(), EGameplayModOp::Additive, Memory.SanityImpact);
	}
}
