#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"

// The interaction HUD and draw helpers live in the runtime module's debug subsystem; these commands
// are the ones that only make sense outside shipping.
static FAutoConsoleCommandWithWorldAndArgs CmdListInteractables(
	TEXT("FV.Interaction.List"),
	TEXT("FV.Interaction.List - log registered interactables and which ones are active."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		const UFVInteractionRegistrySubsystem* Registry = World != nullptr ? World->GetSubsystem<UFVInteractionRegistrySubsystem>() : nullptr;
		if (Registry == nullptr)
		{
			return;
		}

		const TArray<TObjectPtr<UFVInteractableComponent>>& Active = Registry->GetActiveInteractables();

		for (const TObjectPtr<UFVInteractableComponent>& Interactable : Registry->GetAllInteractables())
		{
			UE_LOG(LogFVDebug, Display, TEXT("%s%s"), *GetNameSafe(Interactable != nullptr ? Interactable->GetOwner() : nullptr),
				Active.Contains(Interactable) ? TEXT(" (active)") : TEXT(""));
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdFocus(
	TEXT("FV.Interaction.Focus"),
	TEXT("FV.Interaction.Focus - log what the player is focusing."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		const APawn* Pawn = FVDebug::GetPlayerPawn(World);
		const UFVInteractorComponent* Interactor = Pawn != nullptr ? Pawn->FindComponentByClass<UFVInteractorComponent>() : nullptr;
		const UFVInteractableComponent* Target = Interactor != nullptr ? Interactor->GetTargetInteractable() : nullptr;

		UE_LOG(LogFVDebug, Display, TEXT("target: %s, offers: %d"),
			*GetNameSafe(Target != nullptr ? Target->GetOwner() : nullptr),
			Interactor != nullptr ? Interactor->GetOffers().Num() : 0);
	}));
