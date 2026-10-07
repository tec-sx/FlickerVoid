#include "FVDialogueDebugSubsystem.h"

#include "FVDebugUtils.h"
#include "FVDialogueSubsystem.h"
#include "FVDialogueTypes.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueDebugSubsystem)

static TAutoConsoleVariable<bool> CVarDialogueDebugHUD(
	TEXT("FVCvar.Dialogue.Debug.HUD"),
	false,
	TEXT("Show the running conversation, its speaker and its choices on screen."));

bool UFVDialogueDebugSubsystem::IsEnabled() const
{
	return CVarDialogueDebugHUD.GetValueOnGameThread();
}

void UFVDialogueDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVDialogueSubsystem* Dialogue = UFVDialogueSubsystem::Get(this);
	if (Dialogue == nullptr || !Dialogue->IsInConversation())
	{
		return;
	}

	const FFVDialogueLine& Line = Dialogue->GetCurrentLine();
	
	OutLines.Add(FString::Printf(TEXT("with %s"), *GetNameSafe(Dialogue->GetOwnerActor())));
	OutLines.Add(FString::Printf(TEXT("%s: %s"), *Line.Speaker.GetName(), *Line.Text.ToString()));

	for (const FFVDialogueChoice& Choice : Dialogue->GetChoices())
	{
		OutLines.Add(FString::Printf(TEXT("  [%d] %s%s"), Choice.Index, *Choice.Text.ToString(),
			Choice.bAvailable ? TEXT("") : TEXT(" (locked)")));
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdAdvance(
	TEXT("FV.Dialogue.Advance"),
	TEXT("FV.Dialogue.Advance - skip to the next line."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		if (UFVDialogueSubsystem* Dialogue = UFVDialogueSubsystem::Get(World))
		{
			Dialogue->Advance();
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdChoose(
	TEXT("FV.Dialogue.Choose"),
	TEXT("FV.Dialogue.Choose <Index> - pick a choice."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVDialogueSubsystem* Dialogue = UFVDialogueSubsystem::Get(World);
		if (Dialogue != nullptr && !Args.IsEmpty() && !Dialogue->Choose(FCString::Atoi(*Args[0])))
		{
			UE_LOG(LogFVDebug, Warning, TEXT("Choice %s is not available."), *Args[0]);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdEnd(
	TEXT("FV.Dialogue.End"),
	TEXT("FV.Dialogue.End - abort the conversation."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		if (UFVDialogueSubsystem* Dialogue = UFVDialogueSubsystem::Get(World))
		{
			Dialogue->EndConversation();
		}
	}));
