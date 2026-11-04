#include "Conditions/FVEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVEffect)

#define LOCTEXT_NAMESPACE "FVEffect"

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

FText FFVEffectList::GetSetupError() const
{
	TArray<FText> Errors;
	for (int32 Index = 0; Index < Effects.Num(); ++Index)
	{
		const FFVEffectBase* Ptr = Effects[Index].GetPtr();
		if (Ptr == nullptr)
		{
			Errors.Add(FText::Format(LOCTEXT("NoType", "effect {0} has no type"), Index));
			continue;
		}

		const FText Error = Ptr->GetSetupError();
		if (!Error.IsEmpty())
		{
			const FText Type = FText::FromName(Effects[Index].GetScriptStruct()->GetFName());
			Errors.Add(FText::Format(LOCTEXT("EntryError", "effect {0} ({1}): {2}"), Index, Type, Error));
		}
	}
	return FText::Join(LOCTEXT("ErrorSeparator", "; "), Errors);
}

void UFVEffect::Apply_Implementation(const FFVConditionContext& Context) const
{
}

FText UFVEffect::GetDescription_Implementation() const
{
	return GetClass()->GetDisplayNameText();
}

FText UFVEffect::GetSetupError_Implementation() const
{
	return FText::GetEmpty();
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

FText FFVEffect_Object::GetSetupError() const
{
	if (Effect == nullptr)
	{
		return LOCTEXT("NoObject", "no effect picked");
	}
	return Effect->GetSetupError();
}

#undef LOCTEXT_NAMESPACE
