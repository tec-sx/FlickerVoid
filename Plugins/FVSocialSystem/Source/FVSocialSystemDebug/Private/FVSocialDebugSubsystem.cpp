#include "FVSocialDebugSubsystem.h"

#include "FVDebugUtils.h"
#include "FVFactionDefinition.h"
#include "FVSocialStatics.h"
#include "FVSocialSubsystem.h"
#include "FVSocialTypes.h"
#include "FVTitleDefinition.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialDebugSubsystem)

static TAutoConsoleVariable<bool> CVarSocialDebugHUD(
	TEXT("FVCvar.Social.Debug.HUD"),
	false,
	TEXT("Show faction standing, fame, notoriety and titles on screen."));

bool UFVSocialDebugSubsystem::IsEnabled() const
{
	return CVarSocialDebugHUD.GetValueOnGameThread();
}

void UFVSocialDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const APawn* Pawn = FVDebug::GetPlayerPawn(GetWorld());

	OutLines.Add(FString::Printf(TEXT("Fame %d (read as %d), notoriety %d%s"),
		UFVSocialStatics::GetFame(this),
		UFVSocialStatics::GetRecognizedFame(Pawn),
		UFVSocialStatics::GetGlobalNotoriety(this),
		UFVSocialStatics::IsDisguised(Pawn) ? TEXT(", disguised") : TEXT("")));

	for (const TSoftObjectPtr<UFVFactionDefinition>& Soft : UFVSocialSettings::Get().Factions)
	{
		if (const UFVFactionDefinition* Faction = Soft.Get())
		{
			OutLines.Add(FString::Printf(TEXT("  %s: standing %d, notoriety %d, attitude %d"),
				*Faction->GetName(),
				UFVSocialStatics::GetStanding(this, Faction),
				UFVSocialStatics::GetNotoriety(this, Faction),
				static_cast<int32>(UFVSocialStatics::GetAttitude(Faction, Pawn))));
		}
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
	TEXT("FV.Social.Standing <FactionIdOrAsset> <Delta> - change standing with a faction."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVFactionDefinition* Faction = Args.Num() > 0 ? FVDebug::FindDefinition<UFVFactionDefinition>(Args[0]) : nullptr;
		if (Faction != nullptr)
		{
			UFVSocialStatics::ModifyStanding(World, Faction, Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 10);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdNotoriety(
	TEXT("FV.Social.Notoriety"),
	TEXT("FV.Social.Notoriety <Delta> - change global notoriety."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVSocialStatics::ModifyGlobalNotoriety(World, Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 10);
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdFame(
	TEXT("FV.Social.Fame"),
	TEXT("FV.Social.Fame <Delta> - change fame."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVSocialStatics::ModifyFame(World, Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 10);
	}));
