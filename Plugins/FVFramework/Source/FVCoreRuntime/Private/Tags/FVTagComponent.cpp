#include "Tags/FVTagComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVTagComponent)

UFVTagComponent::UFVTagComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVTagComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(Tags);
}

void UFVTagComponent::AddTag(const FGameplayTag Tag)
{
	Tags.AddTag(Tag);
}

void UFVTagComponent::AddTags(const FGameplayTagContainer& NewTags)
{
	Tags.AppendTags(NewTags);
}

void UFVTagComponent::RemoveTag(const FGameplayTag Tag)
{
	Tags.RemoveTag(Tag);
}
