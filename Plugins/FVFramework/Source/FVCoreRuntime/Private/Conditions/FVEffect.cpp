#include "Conditions/FVEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVEffect)

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

void UFVEffect::Apply_Implementation(const FFVConditionContext& Context) const
{
}

FText UFVEffect::GetDescription_Implementation() const
{
	return GetClass()->GetDisplayNameText();
}

UWorld* UFVEffect::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

void FFVEffect_Object::Apply(const FFVConditionContext& Context) const
{
	if (Effect != nullptr)
	{
		Effect->Apply(Context);
	}
}

FText FFVEffect_Object::GetDescription() const
{
	return Effect != nullptr ? Effect->GetDescription() : FText::GetEmpty();
}
