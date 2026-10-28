#include "Animation/FVAnimNotify_GameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAnimNotify_GameplayEvent)

FString UFVAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	if (!EventTag.IsValid())
	{
		return Super::GetNotifyName_Implementation();
	}

	const FString TagName = EventTag.ToString();
	int32 DotIndex = INDEX_NONE;
	return TagName.FindLastChar(TEXT('.'), DotIndex) ? TagName.RightChop(DotIndex + 1) : TagName;
}

void UFVAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;

	// Editor previews have no ability system; skip them instead of logging an error.
	if (!Owner || !EventTag.IsValid() || !UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}
