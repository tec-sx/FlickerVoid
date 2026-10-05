#include "FVAttributeDebugSubsystem.h"

#include "Attributes/FVAttributeComponent.h"
#include "Checks/FVCheck.h"
#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAttributeDebugSubsystem)

static TAutoConsoleVariable<bool> CVarAttributesDebugHUD(
	TEXT("FVCvar.Attributes.Debug.HUD"),
	false,
	TEXT("Show the player's attributes on screen."));

bool UFVAttributeDebugSubsystem::IsEnabled() const
{
	return CVarAttributesDebugHUD.GetValueOnGameThread();
}

void UFVAttributeDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVAttributeComponent* Attributes = UFVAttributeComponent::Find(FVDebug::GetPlayerPawn(GetWorld()));
	if (Attributes == nullptr)
	{
		return;
	}

	for (const UFVAttributeDefinition* Attribute : Attributes->GetKnownAttributes())
	{
		OutLines.Add(FString::Printf(TEXT("%s = %.1f (base %.1f)"),
			*Attribute->GetName(), Attributes->GetValue(Attribute), Attributes->GetBaseValue(Attribute)));
	}
	OutLines.Sort();
}

static FAutoConsoleCommandWithWorldAndArgs CmdAttributeSet(
	TEXT("FV.Attributes.Set"),
	TEXT("FV.Attributes.Set <AttributeIdOrAsset> <Value> - set a base value on the player."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVAttributeComponent* Attributes = UFVAttributeComponent::Find(FVDebug::GetPlayerPawn(World));
		const UFVAttributeDefinition* Attribute = Args.Num() > 0 ? FVDebug::FindDefinition<UFVAttributeDefinition>(Args[0]) : nullptr;

		if (Attributes != nullptr && Attribute != nullptr)
		{
			Attributes->SetBaseValue(Attribute, Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdCheckRoll(
	TEXT("FV.Attributes.Roll"),
	TEXT("FV.Attributes.Roll <CheckIdOrAsset> [Difficulty] - roll a check as the player and log the breakdown."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVCheckDefinition* Check = Args.Num() > 0 ? FVDebug::FindDefinition<UFVCheckDefinition>(Args[0]) : nullptr;
		if (Check == nullptr)
		{
			return;
		}

		APawn* Pawn = FVDebug::GetPlayerPawn(World);
		const int32 Difficulty = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 10;
		const FFVCheckResult Result = UFVCheckStatics::RollCheck(Check, Pawn, nullptr, Difficulty);

		UE_LOG(LogFVDebug, Display, TEXT("%s: %s (%d vs %d)"), *Check->GetName(),
			Result.bSuccess ? TEXT("success") : TEXT("failure"), Result.Total, Result.Difficulty);

		for (const FFVCheckLine& Line : Result.Breakdown)
		{
			UE_LOG(LogFVDebug, Display, TEXT("  %s %+d"), *Line.Label.ToString(), Line.Value);
		}
	}));
