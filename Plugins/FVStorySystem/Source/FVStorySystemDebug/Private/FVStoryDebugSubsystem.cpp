#include "FVStoryDebugSubsystem.h"

#include "FVDebugUtils.h"
#include "HAL/IConsoleManager.h"
#include "Knowledge/FVKnowledge.h"
#include "Knowledge/FVKnowledgeDefinition.h"
#include "Quest/FVQuest.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVStoryDebugSubsystem)

static TAutoConsoleVariable<bool> CVarStoryDebugHUD(
	TEXT("FVCvar.Story.Debug.HUD"),
	false,
	TEXT("Show active quests and their objectives on screen."));

bool UFVStoryDebugSubsystem::IsEnabled() const
{
	return CVarStoryDebugHUD.GetValueOnGameThread();
}

void UFVStoryDebugSubsystem::CollectLines(TArray<FString>& OutLines) const
{
	const UFVQuestSubsystem* Quests = UFVQuestSubsystem::Get(this);
	if (Quests == nullptr)
	{
		return;
	}

	for (const UFVQuestDefinition* Quest : Quests->GetQuests(EFVQuestState::Active))
	{
		OutLines.Add(Quest->Display.Name.IsEmpty() ? Quest->GetName() : Quest->Display.Name.ToString());

		for (const FFVQuestObjective& Objective : Quest->Objectives)
		{
			if (Quests->IsObjectiveVisible(Objective))
			{
				OutLines.Add(FString::Printf(TEXT("  [%s] %s"),
					Quests->IsObjectiveComplete(Objective) ? TEXT("x") : TEXT(" "),
					*Objective.Description.ToString()));
			}
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs CmdQuestStart(
	TEXT("FV.Quest.Start"),
	TEXT("FV.Quest.Start <QuestIdOrAsset> - start a quest."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVQuestSubsystem* Quests = UFVQuestSubsystem::Get(World);
		UFVQuestDefinition* Quest = Args.Num() > 0 ? FVDebug::FindDefinition<UFVQuestDefinition>(Args[0]) : nullptr;

		if (Quests != nullptr && Quest != nullptr && !Quests->StartQuest(Quest))
		{
			UE_LOG(LogFVDebug, Warning, TEXT("%s could not be started."), *Quest->GetName());
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdQuestComplete(
	TEXT("FV.Quest.Complete"),
	TEXT("FV.Quest.Complete <QuestIdOrAsset> - complete a quest."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVQuestSubsystem* Quests = UFVQuestSubsystem::Get(World);
		UFVQuestDefinition* Quest = Args.Num() > 0 ? FVDebug::FindDefinition<UFVQuestDefinition>(Args[0]) : nullptr;

		if (Quests != nullptr && Quest != nullptr)
		{
			Quests->CompleteQuest(Quest);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs CmdLearn(
	TEXT("FV.Story.Learn"),
	TEXT("FV.Story.Learn <KnowledgeIdOrAsset> - learn a piece of knowledge."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		const UFVKnowledgeDefinition* Knowledge = Args.Num() > 0 ? FVDebug::FindDefinition<UFVKnowledgeDefinition>(Args[0]) : nullptr;
		if (Knowledge != nullptr)
		{
			UFVKnowledgeStatics::Learn(World, Knowledge);
		}
	}));
