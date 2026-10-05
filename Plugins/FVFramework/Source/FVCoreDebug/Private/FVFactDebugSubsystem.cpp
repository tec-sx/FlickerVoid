#include "FVFactDebugSubsystem.h"

#include "FVDebugUtils.h"
#include "Facts/FVFactDatabase.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactDebugSubsystem)

static TAutoConsoleVariable<bool> CVarFactsDebugHUD(
	TEXT("FVCvar.Facts.Debug.HUD"),
	false,
	TEXT("Show facts on screen. Filter with FVCvar.Facts.Debug.Filter."));

static TAutoConsoleVariable<FString> CVarFactsDebugFilter(
	TEXT("FVCvar.Facts.Debug.Filter"),
	TEXT(""),
	TEXT("Only show facts whose tag contains this text."));

bool UFVFactDebugSubsystem::IsEnabled() const
{
	return CVarFactsDebugHUD.GetValueOnGameThread();
}

void UFVFactDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVFactDatabase* Facts = UFVFactDatabase::Get(GetWorld());
	if (Facts == nullptr)
	{
		return;
	}

	const FString Filter = CVarFactsDebugFilter.GetValueOnGameThread();
	for (const TPair<FGameplayTag, int32>& Fact : Facts->GetAllFacts())
	{
		const FString Name = Fact.Key.ToString();
		if (Filter.IsEmpty() || Name.Contains(Filter))
		{
			OutLines.Add(FString::Printf(TEXT("%s = %d"), *Name, Fact.Value));
		}
	}
	OutLines.Sort();
}

static FAutoConsoleCommandWithWorldAndArgs CmdFactSet(
	TEXT("FV.Facts.Set"),
	TEXT("FV.Facts.Set <Tag> <Value>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVFactDatabase* Facts = UFVFactDatabase::Get(World);
		const FGameplayTag Tag = Args.Num() > 0 ? FVDebug::FindTag(Args[0]) : FGameplayTag();
		if (Facts != nullptr && Tag.IsValid())
		{
			Facts->SetFact(Tag, Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 1);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdFactDump(
	TEXT("FV.Facts.Dump"),
	TEXT("FV.Facts.Dump [Filter] - log all facts."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (const UFVFactDatabase* Facts = UFVFactDatabase::Get(World))
		{
			for (const TPair<FGameplayTag, int32>& Fact : Facts->GetAllFacts())
			{
				if (Args.IsEmpty() || Fact.Key.ToString().Contains(Args[0]))
				{
					UE_LOG(LogFVDebug, Display, TEXT("%s = %d"), *Fact.Key.ToString(), Fact.Value);
				}
			}
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdFactReset(
	TEXT("FV.Facts.Reset"),
	TEXT("FV.Facts.Reset - restore default facts."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		if (UFVFactDatabase* Facts = UFVFactDatabase::Get(World))
		{
			Facts->ResetToDefaults();
		}
	}));
