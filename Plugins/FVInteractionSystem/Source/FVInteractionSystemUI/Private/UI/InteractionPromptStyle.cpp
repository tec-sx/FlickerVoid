#include "UI/InteractionPromptStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionPromptStyle)

bool UInteractionPromptStyleAsset::FindStyle(FGameplayTag ActionTag, FInteractionPromptStyle& OutStyle) const
{
	if (const FInteractionPromptStyle* Style = Styles.Find(ActionTag))
	{
		OutStyle = *Style;
		return true;
	}

	OutStyle = FInteractionPromptStyle();
	return false;
}
