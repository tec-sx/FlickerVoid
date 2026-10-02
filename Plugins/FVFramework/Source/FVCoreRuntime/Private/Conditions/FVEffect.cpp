#include "Conditions/FVEffect.h"

void FFVEffectList::Apply(const FFVConditionContext& Context) const
{
	for (const TInstancedStruct<FFVEffectBase>& Effect : Effects)
	{
		if (const FFVEffectBase* Ptr = Effect.GetPtr())
		{
			Ptr->Apply(Context);
		}
	}
}

FText FFVEffectList::GetDescription() const
{
	TArray<FText> Parts;
	for (const TInstancedStruct<FFVEffectBase>& Effect : Effects)
	{
		if (const FFVEffectBase* Ptr = Effect.GetPtr())
		{
			Parts.Add(Ptr->GetDescription());
		}
	}

	return FText::Join(FText::FromString(TEXT(", ")), Parts);
}
