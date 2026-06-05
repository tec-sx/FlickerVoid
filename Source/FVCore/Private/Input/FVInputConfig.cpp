#include "Input/FVInputConfig.h"
#include "InputAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInputConfig)

const UInputAction* UFVInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FFVInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UFVInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FFVInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

TArray<FGameplayTag> UFVInputConfig::GetAllNativeInputTags() const
{
	TArray<FGameplayTag> Tags;
	Tags.Reserve(NativeInputActions.Num());

	for (const FFVInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction)
		{
			Tags.Add(Action.InputTag);
		}
	}
	return Tags;
}

TArray<FGameplayTag> UFVInputConfig::GetAllAbilityInputTags() const
{
	TArray<FGameplayTag> Tags;
	Tags.Reserve(AbilityInputActions.Num());

	for (const FFVInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction)
		{
			Tags.Add(Action.InputTag);
		}
	}
	return Tags;
}
