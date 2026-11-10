#include "Tags/FVTagLibrary.h"

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVTagLibrary)

FGameplayTagContainer UFVTagLibrary::GetActorTags(const AActor* Actor)
{
	FGameplayTagContainer Result;
	if (!Actor)
	{
		return Result;
	}

	TInlineComponentArray<UActorComponent*> Components(Actor);
	for (const UActorComponent* Component : Components)
	{
		const IGameplayTagAssetInterface* TagOwner = Cast<IGameplayTagAssetInterface>(Component);
		if (!TagOwner)
		{
			continue;
		}

		FGameplayTagContainer ComponentTags;
		TagOwner->GetOwnedGameplayTags(ComponentTags);
		Result.AppendTags(ComponentTags);
	}
	return Result;
}

bool UFVTagLibrary::ActorHasTag(const AActor* Actor, const FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	const FGameplayTagContainer Tags = GetActorTags(Actor);
	return Tags.HasTag(Tag);
}

bool UFVTagLibrary::ActorMatchesTags(const AActor* Actor, const FGameplayTagContainer& Required, const FGameplayTagContainer& Blocked, const bool bRequireAll)
{
	const FGameplayTagContainer Tags = GetActorTags(Actor);
	if (Tags.HasAny(Blocked))
	{
		return false;
	}

	if (Required.IsEmpty())
	{
		return true;
	}

	if (bRequireAll)
	{
		return Tags.HasAll(Required);
	}
	return Tags.HasAny(Required);
}
