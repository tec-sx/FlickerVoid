#include "Attributes/FVAttributeComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "FVAttributeSystem.h"
#include "Attributes/FVAttributeDefinition.h"
#include "Attributes/FVAttributeSubsystem.h"
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

	UAbilitySystemComponent* OwnerAbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	const UFVAttributeSubsystem* Subsystem = UFVAttributeSubsystem::Get(this);

	if (Subsystem == nullptr)
	{
		return;
	}

	if (OwnerAbilitySystem == nullptr)
	{
		UE_LOG(LogFVAttributeSystem, Warning, TEXT("%s has an attribute component but no ability system component."), *GetNameSafe(GetOwner()));
		return;
	}

	AbilitySystem = OwnerAbilitySystem;

	for (const UFVAttributeDefinition* Definition : Subsystem->GetDefinitions())
	{
		const FDelegateHandle Handle = OwnerAbilitySystem->GetGameplayAttributeValueChangeDelegate(Definition->Attribute)
			.AddUObject(this, &UFVAttributeComponent::HandleAttributeChanged);
		ChangeHandles.Emplace(Definition, Handle);
	}

	if (UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(GetOwner()))
	{
		Saveable->OnActorDataLoaded.AddUniqueDynamic(this, &UFVAttributeComponent::HandleActorDataLoaded);
	}
}

void UFVAttributeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* OwnerAbilitySystem = AbilitySystem.Get())
	{
		for (const TPair<const UFVAttributeDefinition*, FDelegateHandle>& Pair : ChangeHandles)
		{
			OwnerAbilitySystem->GetGameplayAttributeValueChangeDelegate(Pair.Key->Attribute).Remove(Pair.Value);
		}
	}
	ChangeHandles.Reset();

	Super::EndPlay(EndPlayReason);
}

void UFVAttributeComponent::HandleAttributeChanged(const FOnAttributeChangeData& Data)
{
	const UFVAttributeSubsystem* Subsystem = UFVAttributeSubsystem::Get(this);
	const UFVAttributeDefinition* Definition = Subsystem != nullptr ? Subsystem->FindDefinition(Data.Attribute) : nullptr;
	const UAbilitySystemComponent* OwnerAbilitySystem = AbilitySystem.Get();

	if (Definition == nullptr || OwnerAbilitySystem == nullptr)
	{
		return;
	}

	BaseValues.Add(Definition, OwnerAbilitySystem->GetNumericAttributeBase(Data.Attribute));
	OnAttributeChanged.Broadcast(Definition, Data.OldValue, Data.NewValue);

	UE_LOG(LogFVAttributeSystem, Verbose, TEXT("%s: %s %.2f -> %.2f"), *GetNameSafe(GetOwner()), *GetNameSafe(Definition), Data.OldValue, Data.NewValue);
}

void UFVAttributeComponent::HandleActorDataLoaded()
{
	UAbilitySystemComponent* OwnerAbilitySystem = AbilitySystem.Get();
	if (OwnerAbilitySystem == nullptr)
	{
		return;
	}

	const TMap<TObjectPtr<const UFVAttributeDefinition>, float> Saved = BaseValues;
	for (const TPair<TObjectPtr<const UFVAttributeDefinition>, float>& Pair : Saved)
	{
		if (Pair.Key != nullptr && OwnerAbilitySystem->HasAttributeSetForAttribute(Pair.Key->Attribute))
		{
			OwnerAbilitySystem->SetNumericAttributeBase(Pair.Key->Attribute, Pair.Value);
		}
	}
}
