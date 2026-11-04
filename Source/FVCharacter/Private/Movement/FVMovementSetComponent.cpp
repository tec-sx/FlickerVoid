#include "Movement/FVMovementSetComponent.h"

#include "FVCharacterModule.h"
#include "MovementMode.h"
#include "MovementModeTransition.h"
#include "MoverComponent.h"
#include "Movement/FVMovementSet.h"
#include "Movement/FVMovementSetTransition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMovementSetComponent)

UFVMovementSetComponent* UFVMovementSetComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVMovementSetComponent>() : nullptr;
}

void UFVMovementSetComponent::InstallStartingSets()
{
	UMoverComponent* Mover = GetMover();
	if (!Mover)
	{
		UE_LOG(LogFVMovement, Error, TEXT("%s has movement sets but no Mover component."), *GetNameSafe(GetOwner()));
		return;
	}

	for (const UFVMovementSet* Set : StartingSets)
	{
		if (Set && !HasMovementSet(Set))
		{
			Install(*Mover, *Set, false);
		}
	}

	if (!StartingMovementMode.IsNone())
	{
		Mover->StartingMovementMode = StartingMovementMode;
	}

	if (!Mover->Transitions.ContainsByPredicate([](const UBaseMovementModeTransition* Transition) { return Transition && Transition->IsA<UFVMovementSetTransition>(); }))
	{
		UFVMovementSetTransition* SetTransitions = NewObject<UFVMovementSetTransition>(Mover);
		SetTransitions->bAllowModeReentry = true;
		Mover->Transitions.Add(SetTransitions);
	}
}

bool UFVMovementSetComponent::AddMovementSet(const UFVMovementSet* Set)
{
	UMoverComponent* Mover = GetMover();
	if (!Set || !Mover || HasMovementSet(Set))
	{
		return false;
	}

	Install(*Mover, *Set, HasBegunPlay());
	return true;
}

bool UFVMovementSetComponent::RemoveMovementSet(const UFVMovementSet* Set)
{
	UMoverComponent* Mover = GetMover();
	const int32 Index = InstalledSets.IndexOfByPredicate([Set](const FFVInstalledMovementSet& Installed) { return Installed.Set == Set; });
	if (!Mover || Index == INDEX_NONE)
	{
		return false;
	}

	const FFVInstalledMovementSet& Installed = InstalledSets[Index];
	for (const FName& ModeName : Installed.ModeNames)
	{
		Mover->RemoveMovementMode(ModeName);
	}

	for (UBaseMovementModeTransition* Transition : Installed.Transitions)
	{
		Transition->OnUnregistered();
	}

	InstalledSets.RemoveAt(Index);
	return true;
}

bool UFVMovementSetComponent::HasMovementSet(const UFVMovementSet* Set) const
{
	return InstalledSets.ContainsByPredicate([Set](const FFVInstalledMovementSet& Installed) { return Installed.Set == Set; });
}

UMoverComponent* UFVMovementSetComponent::GetMover() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UMoverComponent>() : nullptr;
}

void UFVMovementSetComponent::Install(UMoverComponent& Mover, const UFVMovementSet& Set, bool bMoverInitialized)
{
	FFVInstalledMovementSet& Installed = InstalledSets.AddDefaulted_GetRef();
	Installed.Set = &Set;

	// Mover owns its modes and transitions, so each character gets its own copies of the set's templates.
	for (const FFVMovementSetMode& Entry : Set.Modes)
	{
		if (Entry.Name.IsNone() || !Entry.Mode)
		{
			continue;
		}

		UBaseMovementMode* Mode = DuplicateObject<UBaseMovementMode>(Entry.Mode, &Mover);
		if (bMoverInitialized)
		{
			Mover.RemoveMovementMode(Entry.Name);
			Mover.AddMovementModeFromObject(Entry.Name, Mode);
		}
		else
		{
			Mover.MovementModes.Add(Entry.Name, Mode);
		}

		Installed.ModeNames.Add(Entry.Name);
	}

	// UFVMovementSetTransition runs these, so they also work for sets added after Mover initialized.
	for (const UBaseMovementModeTransition* Template : Set.Transitions)
	{
		if (Template)
		{
			UBaseMovementModeTransition* Transition = DuplicateObject<UBaseMovementModeTransition>(Template, &Mover);
			Transition->OnRegistered();
			Installed.Transitions.Add(Transition);
		}
	}
}
