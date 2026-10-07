#include "Attributes/FVAttributeComponent.h"

#include "FVAttributeSettings.h"
#include "FVAttributeSystem.h"
#include "Attributes/FVAttributeFragments.h"
#include "Identity/FVIdentityComponent.h"
#include "GameFramework/Actor.h"
#include "Save/FVSaveableComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeComponent)

UFVAttributeComponent::UFVAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVAttributeComponent* UFVAttributeComponent::Find(const AActor* Actor)
{
	return Actor != nullptr ? Actor->FindComponentByClass<UFVAttributeComponent>() : nullptr;
}

void UFVAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplySet(StartingSet != nullptr ? StartingSet.Get() : UFVAttributeSettings::Get().DefaultAttributeSet.LoadSynchronous());

	if (const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(GetOwner()))
	{
		if (const FFVCharacterFragment_Attributes* Fragment = Definition->FindFragment<FFVCharacterFragment_Attributes>())
		{
			ApplySet(Fragment->AttributeSet);

			for (const FFVAttributeValue& Value : Fragment->Overrides)
			{
				SetBaseValue(Value.Attribute, Value.Value);
			}
		}
	}

	for (const FFVAttributeValue& Value : StartingValues)
	{
		SetBaseValue(Value.Attribute, Value.Value);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVAttributeComponent::HandleActorDataLoaded);
	}
}

void UFVAttributeComponent::HandleActorDataLoaded()
{
	TArray<TObjectPtr<const UFVAttributeDefinition>> Attributes;
	BaseValues.GetKeys(Attributes);
	for (const UFVAttributeDefinition* Attribute : Attributes)
	{
		Recompute(Attribute);
	}
}

void UFVAttributeComponent::ApplySet(const UFVAttributeSetDefinition* Set)
{
	if (Set == nullptr)
	{
		return;
	}

	TArray<FFVAttributeValue> Values;
	Set->Collect(Values);

	for (const FFVAttributeValue& Value : Values)
	{
		SetBaseValue(Value.Attribute, Value.Value);
	}
}

float UFVAttributeComponent::GetValue(const UFVAttributeDefinition* Attribute) const
{
	if (Attribute == nullptr)
	{
		return 0.f;
	}

	if (const float* Current = CurrentValues.Find(Attribute))
	{
		return *Current;
	}

	return Attribute->Clamp(Attribute->DefaultValue);
}

float UFVAttributeComponent::GetBaseValue(const UFVAttributeDefinition* Attribute) const
{
	if (Attribute == nullptr)
	{
		return 0.f;
	}

	const float* Base = BaseValues.Find(Attribute);
	return Attribute->Clamp(Base != nullptr ? *Base : Attribute->DefaultValue);
}

void UFVAttributeComponent::SetBaseValue(const UFVAttributeDefinition* Attribute, float Value)
{
	if (Attribute == nullptr)
	{
		return;
	}

	BaseValues.Add(Attribute, Attribute->Clamp(Value));
	Recompute(Attribute);
}

void UFVAttributeComponent::ModifyBaseValue(const UFVAttributeDefinition* Attribute, float Delta)
{
	if (Attribute != nullptr)
	{
		SetBaseValue(Attribute, GetBaseValue(Attribute) + Delta);
	}
}

void UFVAttributeComponent::AddModifier(const FFVAttributeModifier& Modifier)
{
	if (Modifier.Attribute == nullptr)
	{
		return;
	}

	Modifiers.Add(Modifier);
	Recompute(Modifier.Attribute);
}

void UFVAttributeComponent::RemoveModifiersFromSource(FGameplayTag Source)
{
	TArray<const UFVAttributeDefinition*> Touched;

	for (int32 Index = Modifiers.Num() - 1; Index >= 0; --Index)
	{
		if (Modifiers[Index].Source == Source)
		{
			Touched.AddUnique(Modifiers[Index].Attribute);
			Modifiers.RemoveAt(Index);
		}
	}

	for (const UFVAttributeDefinition* Attribute : Touched)
	{
		Recompute(Attribute);
	}
}

TArray<UFVAttributeDefinition*> UFVAttributeComponent::GetKnownAttributes() const
{
	TArray<UFVAttributeDefinition*> Out;
	for (const TPair<TObjectPtr<const UFVAttributeDefinition>, float>& Pair : BaseValues)
	{
		if (Pair.Key != nullptr)
		{
			Out.Add(const_cast<UFVAttributeDefinition*>(Pair.Key.Get()));
		}
	}
	return Out;
}

void UFVAttributeComponent::Recompute(const UFVAttributeDefinition* Attribute)
{
	if (Attribute == nullptr)
	{
		return;
	}

	float Value = GetBaseValue(Attribute);
	float Multiplier = 1.f;
	bool bOverridden = false;
	float Override = 0.f;

	for (const FFVAttributeModifier& Modifier : Modifiers)
	{
		if (Modifier.Attribute != Attribute)
		{
			continue;
		}

		switch (Modifier.Op)
		{
		case EFVAttributeModifierOp::Add:
			Value += Modifier.Value;
			break;
		case EFVAttributeModifierOp::Multiply:
			Multiplier *= Modifier.Value;
			break;
		case EFVAttributeModifierOp::Override:
			bOverridden = true;
			Override = Modifier.Value;
			break;
		}
	}

	const float NewValue = Attribute->Clamp(bOverridden ? Override : Value * Multiplier);
	const float OldValue = GetValue(Attribute);

	if (FMath::IsNearlyEqual(OldValue, NewValue) && CurrentValues.Contains(Attribute))
	{
		return;
	}

	CurrentValues.Add(Attribute, NewValue);
	OnAttributeChanged.Broadcast(Attribute, OldValue, NewValue);

	UE_LOG(LogFVAttributeSystem, Verbose, TEXT("%s: %s %.2f -> %.2f"), *GetNameSafe(GetOwner()), *GetNameSafe(Attribute), OldValue, NewValue);
}
