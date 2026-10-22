#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Core/FVInteractionGameplayTags.h"
#include "Relevance/FVRelevanceComponent.h"
#include "Relevance/FVRelevanceSubsystem.h"

// The interaction HUD and draw helpers live in the runtime module's debug subsystem; these commands
// are the ones that only make sense outside shipping.
static FAutoConsoleCommandWithWorldAndArgs CmdListInteractables(
	TEXT("FV.Interaction.List"),
	TEXT("FV.Interaction.List - log interactables by distance with their relevance band and state."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		const UFVRelevanceSubsystem* Relevance = UFVRelevanceSubsystem::Get(World);
		if (Relevance == nullptr)
		{
			return;
		}

		TArray<UFVRelevanceComponent*> Components;
		Relevance->GetComponents(FVInteractionGameplayTags::Relevance_Interactable, Components);

		for (const UFVRelevanceComponent* Component : Components)
		{
			const AActor* Owner = Component->GetOwner();
			const UFVInteractableComponent* Interactable = Owner != nullptr ? Owner->FindComponentByClass<UFVInteractableComponent>() : nullptr;

			UE_LOG(LogFVDebug, Display, TEXT("%s: %s, %.0f cm, %s"), *GetNameSafe(Owner),
				*UEnum::GetDisplayValueAsText(Component->GetBand()).ToString(), Component->GetDistance(),
				Interactable != nullptr ? *UEnum::GetDisplayValueAsText(Interactable->GetState()).ToString() : TEXT("no interactable"));
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
