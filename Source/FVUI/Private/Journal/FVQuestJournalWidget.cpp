#include "Journal/FVQuestJournalWidget.h"

UFVQuestSubsystem* UFVQuestJournalWidget::GetQuestSubsystem() const
{
	return UFVQuestSubsystem::Get(this);
}

void UFVQuestJournalWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UFVQuestSubsystem* Quests = GetQuestSubsystem())
	{
		Quests->OnQuestsChanged.AddUniqueDynamic(this, &UFVQuestJournalWidget::HandleQuestsChanged);
	}
	OnJournalChanged();
}

void UFVQuestJournalWidget::NativeDestruct()
{
	if (UFVQuestSubsystem* Quests = GetQuestSubsystem())
	{
		Quests->OnQuestsChanged.RemoveDynamic(this, &UFVQuestJournalWidget::HandleQuestsChanged);
	}
	Super::NativeDestruct();
}

TArray<UFVQuestDefinition*> UFVQuestJournalWidget::GetQuests(EFVQuestState State) const
{
	const UFVQuestSubsystem* Quests = GetQuestSubsystem();
	return Quests ? Quests->GetQuests(State) : TArray<UFVQuestDefinition*>();
}

TArray<FFVJournalObjective> UFVQuestJournalWidget::GetVisibleObjectives(const UFVQuestDefinition* Quest) const
{
	TArray<FFVJournalObjective> Result;
	const UFVQuestSubsystem* Quests = GetQuestSubsystem();
	if (!Quests || !Quest)
	{
		return Result;
	}

	for (const FFVQuestObjective& Objective : Quest->Objectives)
	{
		if (!Quests->IsObjectiveVisible(Objective))
		{
			continue;
		}
		FFVJournalObjective& Row = Result.AddDefaulted_GetRef();
		Row.Description = Objective.Description;
		Row.bComplete = Quests->IsObjectiveComplete(Objective);
		Row.bOptional = Objective.bOptional;
	}
	return Result;
}

EFVQuestState UFVQuestJournalWidget::GetQuestState(const UFVQuestDefinition* Quest) const
{
	const UFVQuestSubsystem* Quests = GetQuestSubsystem();
	return Quests ? Quests->GetState(Quest) : EFVQuestState::NotStarted;
}

void UFVQuestJournalWidget::SelectQuest(UFVQuestDefinition* Quest)
{
	if (SelectedQuest == Quest)
	{
		return;
	}
	SelectedQuest = Quest;
	OnJournalChanged();
}

void UFVQuestJournalWidget::HandleQuestsChanged()
{
	OnJournalChanged();
}