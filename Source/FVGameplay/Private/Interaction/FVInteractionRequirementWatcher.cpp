#include "Interaction/FVInteractionRequirementWatcher.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/InteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionRequirementWatcher)

void UFVInteractionRequirementWatcher::BindResponses_Implementation(UInteractionResponseComponent* Response)
{
	AActor* OwningActor = GetOwner();
	Interactor = OwningActor->FindComponentByClass<UInteractorComponent>();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
	if (!ASC || !Interactor.IsValid())
	{
		return;
	}

	for (const FGameplayTag& Tag : WatchedTags)
	{
		TagHandles.Add(ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
			.AddWeakLambda(this, [this](const FGameplayTag&, int32) { RequestRefresh(); }));
	}

	for (const FGameplayAttribute& Attribute : WatchedAttributes)
	{
		if (!Attribute.IsValid())
		{
			continue;
		}

		const FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
			.AddWeakLambda(this, [this](const FOnAttributeChangeData&) { RequestRefresh(); });

		AttributeHandles.Emplace(Attribute, Handle);
	}
}

void UFVInteractionRequirementWatcher::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		TArray<FGameplayTag> Tags;
		WatchedTags.GetGameplayTagArray(Tags);

		for (int32 Index = 0; Index < TagHandles.Num() && Index < Tags.Num(); ++Index)
		{
			ASC->RegisterGameplayTagEvent(Tags[Index], EGameplayTagEventType::NewOrRemoved)
				.Remove(TagHandles[Index]);
		}

		for (const TPair<FGameplayAttribute, FDelegateHandle>& Binding : AttributeHandles)
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Binding.Key).Remove(Binding.Value);
		}
	}

	TagHandles.Reset();
	AttributeHandles.Reset();

	Super::EndPlay(EndPlayReason);
}

void UFVInteractionRequirementWatcher::RequestRefresh()
{
	if (UInteractorComponent* InteractorPtr = Interactor.Get())
	{
		InteractorPtr->RequestOfferRefresh();
	}
}
