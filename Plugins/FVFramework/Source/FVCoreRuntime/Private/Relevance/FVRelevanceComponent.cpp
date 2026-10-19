#include "Relevance/FVRelevanceComponent.h"

#include "FVCoreRuntime.h"
#include "GameFramework/Actor.h"
#include "Relevance/FVRelevanceSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRelevanceComponent)

UFVRelevanceComponent::UFVRelevanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVRelevanceComponent* UFVRelevanceComponent::Find(const AActor* Actor, const FGameplayTag& InGroup)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	TInlineComponentArray<UFVRelevanceComponent*> Components(Actor);
	for (UFVRelevanceComponent* Component : Components)
	{
		if (Component->Group == InGroup)
		{
			return Component;
		}
	}
	return nullptr;
}

UFVRelevanceComponent* UFVRelevanceComponent::FindOrAdd(AActor* Actor, const FGameplayTag& InGroup)
{
	if (Actor == nullptr || !InGroup.IsValid())
	{
		return nullptr;
	}

	if (UFVRelevanceComponent* Existing = Find(Actor, InGroup))
	{
		return Existing;
	}

	UFVRelevanceComponent* Component = NewObject<UFVRelevanceComponent>(Actor, MakeUniqueObjectName(Actor, StaticClass(), TEXT("Relevance")));
	Component->Group = InGroup;
	Actor->AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

void UFVRelevanceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Group.IsValid())
	{
		return;
	}

	if (const FFVRelevanceGroup* Settings = UFVRelevanceSettings::Get().FindGroup(Group))
	{
		GroupSettings = *Settings;
	}
	else
	{
		UE_LOG(LogFVCore, Warning, TEXT("%s: relevance group '%s' has no thresholds in FlickerVoid > Relevance; using defaults."), *GetNameSafe(GetOwner()), *Group.ToString());
	}

	if (UFVRelevanceSubsystem* Subsystem = UFVRelevanceSubsystem::Get(this))
	{
		bRegistered = Subsystem->Register(this);
	}
}

void UFVRelevanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegistered)
	{
		if (UFVRelevanceSubsystem* Subsystem = UFVRelevanceSubsystem::Get(this))
		{
			Subsystem->Unregister(this);
		}
		bRegistered = false;
	}

	Super::EndPlay(EndPlayReason);
}

float UFVRelevanceComponent::ScoreViewpoint(const FTransform& Viewpoint) const
{
	const AActor* Owner = GetOwner();
	return Owner != nullptr ? -FVector::Dist(Owner->GetActorLocation(), Viewpoint.GetLocation()) : -TNumericLimits<float>::Max();
}

bool UFVRelevanceComponent::UpdateBand(const float NewDistance)
{
	Distance = NewDistance;
	Band = GroupSettings.ComputeBand(Distance, Band);
	return Band != BroadcastBand;
}

void UFVRelevanceComponent::BroadcastBandChange()
{
	if (Band != BroadcastBand)
	{
		const EFVRelevanceBand OldBand = BroadcastBand;
		BroadcastBand = Band;
		OnBandChanged.Broadcast(Band, OldBand);
	}
}
