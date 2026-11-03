#include "Social/FVSocialDebug.h"

#include "FVDebugUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Scales/FVScaleDefinition.h"
#include "Scales/FVScaleLibrary.h"
#include "Social/FVFactionDefinition.h"
#include "Social/FVSocialLibrary.h"
#include "Social/FVSocialSubsystem.h"
#include "Social/FVTitleDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialDebug)

static TAutoConsoleVariable<bool> CVarSocialDebugHUD(
	TEXT("FVCvar.Social.Debug.HUD"),
	false,
	TEXT("Show faction standing, fame, notoriety and titles on screen."));

namespace FVSocialDebug
{
	void Modify(UWorld* World, const UFVScaleDefinition* Scale, const TArray<FString>& Args, const int32 DeltaArg)
	{
		const int32 Delta = Args.IsValidIndex(DeltaArg) ? FCString::Atoi(*Args[DeltaArg]) : 10;
		UFVScaleLibrary::ModifyScale(World, Scale, Delta);
	}
}

bool UFVSocialDebugSubsystem::IsEnabled() const
{
	return CVarSocialDebugHUD.GetValueOnGameThread();
}

void UFVSocialDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const APawn* Pawn = FVDebug::GetPlayerPawn(GetWorld());
	const UFVScaleDefinition* Fame = UFVSocialLibrary::GetFameScale();
	const UFVScaleDefinition* Notoriety = UFVSocialLibrary::GetNotorietyScale();

	const int32 FameValue = UFVScaleLibrary::GetValue(this, Fame);
	const int32 RecognizedFame = UFVSocialLibrary::GetRecognizedFame(Pawn);
	const int32 NotorietyValue = UFVScaleLibrary::GetValue(this, Notoriety);
	const bool bDisguised = UFVSocialLibrary::IsDisguised(Pawn);
	OutLines.Add(FString::Printf(TEXT("Fame %d (read as %d), notoriety %d%s"),
		FameValue, RecognizedFame, NotorietyValue, bDisguised ? TEXT(", disguised") : TEXT("")));

	for (const UFVFactionDefinition* Faction : UFVSocialLibrary::GetFactions())
	{
		const int32 Standing = UFVScaleLibrary::GetValue(this, Faction->Standing);
		const int32 FactionNotoriety = UFVScaleLibrary::GetValue(this, Faction->Notoriety);
		const EFVAttitude Attitude = UFVSocialLibrary::GetAttitude(Faction, Pawn);
		const FString AttitudeName = UEnum::GetDisplayValueAsText(Attitude).ToString();
		OutLines.Add(FString::Printf(TEXT("  %s: standing %d, notoriety %d, %s"), *Faction->GetName(), Standing, FactionNotoriety, *AttitudeName));
	}

	if (const UFVSocialSubsystem* Social = UFVSocialSubsystem::Get(this))
	{
		for (const UFVTitleDefinition* Title : Social->GetHeldTitles())
		{
			OutLines.Add(FString::Printf(TEXT("  title: %s"), *Title->GetName()));
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdStanding(
	TEXT("FV.Social.Standing"),
	TEXT("FV.Social.Standing <FactionIdOrAsset> <Delta> - change standing with a faction, passing the change on through its links."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVFactionDefinition* Faction = Args.Num() > 0 ? FVDebug::FindDefinition<UFVFactionDefinition>(Args[0]) : nullptr;
		if (Faction != nullptr)
		{
			FVSocialDebug::Modify(World, Faction->Standing, Args, 1);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdNotoriety(
	TEXT("FV.Social.Notoriety"),
	TEXT("FV.Social.Notoriety <Delta> - change global notoriety."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVScaleDefinition* Notoriety = UFVSocialLibrary::GetNotorietyScale();
		FVSocialDebug::Modify(World, Notoriety, Args, 0);
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdFame(
	TEXT("FV.Social.Fame"),
	TEXT("FV.Social.Fame <Delta> - change fame."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVScaleDefinition* Fame = UFVSocialLibrary::GetFameScale();
		FVSocialDebug::Modify(World, Fame, Args, 0);
	}));
