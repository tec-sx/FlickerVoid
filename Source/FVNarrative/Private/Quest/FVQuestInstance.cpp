// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest/FVQuestInstance.h"
#include "Quest/FVQuestDefinition.h"
#include "Quest/FVQuestObjective.h"
#include "Subsystems/FVWorldStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestInstance)

UFVQuestInstance::UFVQuestInstance()
{
	QuestDefinition = nullptr;
	TimeStarted = 0.0f;
	TimeEnded = 0.0f;
	bIsTracked = false;
}

void UFVQuestInstance::InitializeFromDefinition(const UFVQuestDefinition* Definition)
{
	if (!Definition)
	{
		return;
	}

	QuestDefinition = Definition;
	CurrentState = FGameplayTag::RequestGameplayTag(FName("Quest.State.Available"));

	// Initialize objective progress
	ObjectiveProgress.Empty();
	for (const UFVQuestObjective* Objective : Definition->Objectives)
	{
		if (Objective)
		{
			FFVObjectiveProgress Progress;
			Progress.ObjectiveId = Objective->ObjectiveId;
			Progress.State = Objective->RequiredObjectiveIds.Num() > 0 ? EFVObjectiveState::Locked : EFVObjectiveState::Locked;
			Progress.CurrentCount = 0;
			Progress.TimeStarted = 0.0f;
			Progress.TimeCompleted = 0.0f;
			ObjectiveProgress.Add(Progress);
		}
	}
}

void UFVQuestInstance::StartQuest()
{
	if (!QuestDefinition)
	{
		return;
	}

	SetState(FGameplayTag::RequestGameplayTag(FName("Quest.State.Active")));
	
	// Get current game time
	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			TimeStarted = WorldState->GetCurrentGameTime();
		}
	}

	// Activate initial objectives
	UpdateObjectiveStates();

	// Apply start consequences
	ApplyConsequences(QuestDefinition->OnStartConsequences);
}

void UFVQuestInstance::CompleteQuest()
{
	if (!QuestDefinition)
	{
		return;
	}

	SetState(FGameplayTag::RequestGameplayTag(FName("Quest.State.Completed")));

	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			TimeEnded = WorldState->GetCurrentGameTime();
		}
	}

	ApplyConsequences(QuestDefinition->OnCompleteConsequences);
}

void UFVQuestInstance::FailQuest()
{
	if (!QuestDefinition)
	{
		return;
	}

	SetState(FGameplayTag::RequestGameplayTag(FName("Quest.State.Failed")));

	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			TimeEnded = WorldState->GetCurrentGameTime();
		}
	}

	ApplyConsequences(QuestDefinition->OnFailConsequences);
}

void UFVQuestInstance::AbandonQuest()
{
	if (!QuestDefinition || !QuestDefinition->bCanAbandon)
	{
		return;
	}

	SetState(FGameplayTag::RequestGameplayTag(FName("Quest.State.Abandoned")));

	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			TimeEnded = WorldState->GetCurrentGameTime();
		}
	}

	ApplyConsequences(QuestDefinition->OnAbandonConsequences);
}

void UFVQuestInstance::UpdateObjectiveProgress(FName ObjectiveId, int32 Delta)
{
	FFVObjectiveProgress* Progress = GetObjectiveProgress(ObjectiveId);
	if (!Progress || Progress->State != EFVObjectiveState::Active)
	{
		return;
	}

	Progress->CurrentCount = FMath::Max(0, Progress->CurrentCount + Delta);

	OnObjectiveStateChanged.Broadcast(this, ObjectiveId);

	// Check if objective is now complete
	if (QuestDefinition)
	{
		for (const UFVQuestObjective* Objective : QuestDefinition->Objectives)
		{
			if (Objective && Objective->ObjectiveId == ObjectiveId)
			{
				if (Objective->CheckCompletion(this))
				{
					CompleteObjective(ObjectiveId);
				}
				break;
			}
		}
	}
}

void UFVQuestInstance::CompleteObjective(FName ObjectiveId)
{
	FFVObjectiveProgress* Progress = GetObjectiveProgress(ObjectiveId);
	if (!Progress || Progress->State != EFVObjectiveState::Active)
	{
		return;
	}

	Progress->State = EFVObjectiveState::Completed;

	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			Progress->TimeCompleted = WorldState->GetCurrentGameTime();
		}
	}

	OnObjectiveStateChanged.Broadcast(this, ObjectiveId);

	// Update other objective states and check quest completion
	UpdateObjectiveStates();
	CheckQuestCompletion();
}

void UFVQuestInstance::FailObjective(FName ObjectiveId)
{
	FFVObjectiveProgress* Progress = GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return;
	}

	Progress->State = EFVObjectiveState::Failed;
	OnObjectiveStateChanged.Broadcast(this, ObjectiveId);

	// Check if quest should fail
	if (QuestDefinition)
	{
		for (const UFVQuestObjective* Objective : QuestDefinition->Objectives)
		{
			if (Objective && Objective->ObjectiveId == ObjectiveId && !Objective->bOptional)
			{
				// Non-optional objective failed - fail the quest
				FailQuest();
				return;
			}
		}
	}
}

FFVObjectiveProgress* UFVQuestInstance::GetObjectiveProgress(FName ObjectiveId)
{
	for (FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.ObjectiveId == ObjectiveId)
		{
			return &Progress;
		}
	}
	return nullptr;
}

FFVObjectiveProgress UFVQuestInstance::GetObjectiveProgressCopy(FName ObjectiveId) const
{
	for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.ObjectiveId == ObjectiveId)
		{
			return Progress;
		}
	}
	return FFVObjectiveProgress();
}

bool UFVQuestInstance::IsObjectiveActive(FName ObjectiveId) const
{
	for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.ObjectiveId == ObjectiveId)
		{
			return Progress.State == EFVObjectiveState::Active;
		}
	}
	return false;
}

bool UFVQuestInstance::IsObjectiveComplete(FName ObjectiveId) const
{
	for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.ObjectiveId == ObjectiveId)
		{
			return Progress.State == EFVObjectiveState::Completed;
		}
	}
	return false;
}

TArray<FName> UFVQuestInstance::GetActiveObjectiveIds() const
{
	TArray<FName> ActiveIds;
	for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.State == EFVObjectiveState::Active)
		{
			ActiveIds.Add(Progress.ObjectiveId);
		}
	}
	return ActiveIds;
}

float UFVQuestInstance::GetOverallProgress() const
{
	if (ObjectiveProgress.Num() == 0)
	{
		return 0.0f;
	}

	int32 CompletedCount = 0;
	int32 TotalRequired = 0;

	for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		// Find the objective definition to check if it's optional
		bool bIsOptional = false;
		if (QuestDefinition)
		{
			for (const UFVQuestObjective* Objective : QuestDefinition->Objectives)
			{
				if (Objective && Objective->ObjectiveId == Progress.ObjectiveId)
				{
					bIsOptional = Objective->bOptional;
					break;
				}
			}
		}

		if (!bIsOptional)
		{
			TotalRequired++;
			if (Progress.State == EFVObjectiveState::Completed)
			{
				CompletedCount++;
			}
		}
	}

	if (TotalRequired == 0)
	{
		return 1.0f;
	}

	return static_cast<float>(CompletedCount) / static_cast<float>(TotalRequired);
}

bool UFVQuestInstance::IsComplete() const
{
	return CurrentState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Completed")));
}

bool UFVQuestInstance::IsFailed() const
{
	return CurrentState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Failed")));
}

bool UFVQuestInstance::IsActive() const
{
	return CurrentState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Active")));
}

void UFVQuestInstance::SetCustomData(FName Key, const FString& Value)
{
	CustomData.Add(Key, Value);
}

FString UFVQuestInstance::GetCustomData(FName Key) const
{
	const FString* Value = CustomData.Find(Key);
	return Value ? *Value : FString();
}

void UFVQuestInstance::SetState(FGameplayTag NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnQuestStateChanged.Broadcast(this, NewState);
	}
}

void UFVQuestInstance::UpdateObjectiveStates()
{
	if (!QuestDefinition)
	{
		return;
	}

	float CurrentTime = 0.0f;
	if (UWorld* World = GetWorld())
	{
		if (UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>())
		{
			CurrentTime = WorldState->GetCurrentGameTime();
		}
	}

	for (FFVObjectiveProgress& Progress : ObjectiveProgress)
	{
		if (Progress.State != EFVObjectiveState::Locked)
		{
			continue;
		}

		// Find the objective definition
		const UFVQuestObjective* ObjectiveDef = nullptr;
		for (const UFVQuestObjective* Objective : QuestDefinition->Objectives)
		{
			if (Objective && Objective->ObjectiveId == Progress.ObjectiveId)
			{
				ObjectiveDef = Objective;
				break;
			}
		}

		if (!ObjectiveDef)
		{
			continue;
		}

		// Check if all required objectives are complete
		bool bCanActivate = true;
		for (const FName& RequiredId : ObjectiveDef->RequiredObjectiveIds)
		{
			if (!IsObjectiveComplete(RequiredId))
			{
				bCanActivate = false;
				break;
			}
		}

		if (bCanActivate)
		{
			Progress.State = EFVObjectiveState::Active;
			Progress.TimeStarted = CurrentTime;
			OnObjectiveStateChanged.Broadcast(this, Progress.ObjectiveId);
		}
	}
}

void UFVQuestInstance::CheckQuestCompletion()
{
	if (!QuestDefinition || !IsActive())
	{
		return;
	}

	bool bShouldComplete = false;

	switch (QuestDefinition->CompletionType)
	{
	case EFVQuestCompletionType::AllObjectives:
		{
			bShouldComplete = true;
			for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
			{
				// Find if objective is optional
				bool bIsOptional = false;
				for (const UFVQuestObjective* Objective : QuestDefinition->Objectives)
				{
					if (Objective && Objective->ObjectiveId == Progress.ObjectiveId)
					{
						bIsOptional = Objective->bOptional;
						break;
					}
				}

				if (!bIsOptional && Progress.State != EFVObjectiveState::Completed)
				{
					bShouldComplete = false;
					break;
				}
			}
		}
		break;

	case EFVQuestCompletionType::AnyObjective:
		{
			for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
			{
				if (Progress.State == EFVObjectiveState::Completed)
				{
					bShouldComplete = true;
					break;
				}
			}
		}
		break;

	case EFVQuestCompletionType::MinimumObjectives:
		{
			int32 CompletedCount = 0;
			for (const FFVObjectiveProgress& Progress : ObjectiveProgress)
			{
				if (Progress.State == EFVObjectiveState::Completed)
				{
					CompletedCount++;
				}
			}
			bShouldComplete = CompletedCount >= QuestDefinition->MinimumObjectivesRequired;
		}
		break;

	case EFVQuestCompletionType::Custom:
		// Custom completion logic handled elsewhere
		break;
	}

	if (bShouldComplete)
	{
		CompleteQuest();
	}
}

void UFVQuestInstance::ApplyConsequences(const FFVQuestConsequence& Consequences)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UFVWorldStateSubsystem* WorldState = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>();
	if (!WorldState)
	{
		return;
	}

	// Add world state tags
	for (const FGameplayTag& Tag : Consequences.WorldStateTags)
	{
		WorldState->AddWorldStateTag(Tag);
	}

	// Update NPC relationships
	for (const auto& RelationshipChange : Consequences.RelationshipChanges)
	{
		WorldState->ModifyNPCRelationship(RelationshipChange.Key, RelationshipChange.Value);
	}

	// TODO: Apply gameplay effects to protagonist
	// TODO: Unlock memories
	// TODO: Unlock/block quests
}