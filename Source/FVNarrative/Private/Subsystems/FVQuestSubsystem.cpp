// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/FVQuestSubsystem.h"
#include "Quest/FVQuestDefinition.h"
#include "Quest/FVQuestInstance.h"
#include "Quest/FVQuestObjective.h"
#include "Subsystems/FVWorldStateSubsystem.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestSubsystem)

void UFVQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Ensure WorldStateSubsystem is initialized first
	Collection.InitializeDependency<UFVWorldStateSubsystem>();
}

void UFVQuestSubsystem::Deinitialize()
{
	ActiveQuests.Empty();
	CompletedQuestIds.Empty();
	FailedQuestIds.Empty();
	TrackedQuest.Reset();

	Super::Deinitialize();
}

UFVQuestInstance* UFVQuestSubsystem::StartQuest(const UFVQuestDefinition* Definition)
{
	if (!Definition)
	{
		return nullptr;
	}

	// Check if quest is already active
	if (ActiveQuests.Contains(Definition->QuestId))
	{
		return ActiveQuests[Definition->QuestId];
	}

	// Check if quest was already completed and isn't repeatable
	if (!Definition->bRepeatable && CompletedQuestIds.Contains(Definition->QuestId))
	{
		return nullptr;
	}

	// Check prerequisites
	if (!IsQuestAvailable(Definition))
	{
		return nullptr;
	}

	// Create new quest instance
	UFVQuestInstance* NewQuest = NewObject<UFVQuestInstance>(this);
	NewQuest->InitializeFromDefinition(Definition);
	NewQuest->StartQuest();

	// Bind to state changes
	NewQuest->OnQuestStateChanged.AddDynamic(this, &UFVQuestSubsystem::OnQuestStateChanged);

	ActiveQuests.Add(Definition->QuestId, NewQuest);

	// Auto-track if no quest is currently tracked
	if (!TrackedQuest.IsValid() && Definition->bTrackable)
	{
		SetTrackedQuest(NewQuest);
	}

	OnQuestStarted.Broadcast(NewQuest);

	return NewQuest;
}

UFVQuestInstance* UFVQuestSubsystem::StartQuestById(FPrimaryAssetId QuestAssetId)
{
	// Load the quest definition
	UAssetManager& AssetManager = UAssetManager::Get();
	
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(QuestAssetId);
	if (!AssetPath.IsValid())
	{
		return nullptr;
	}

	const UFVQuestDefinition* Definition = Cast<UFVQuestDefinition>(AssetPath.TryLoad());
	return StartQuest(Definition);
}

UFVQuestInstance* UFVQuestSubsystem::GetQuestInstance(FName QuestId) const
{
	const TObjectPtr<UFVQuestInstance>* Found = ActiveQuests.Find(QuestId);
	return Found ? *Found : nullptr;
}

TArray<UFVQuestInstance*> UFVQuestSubsystem::GetActiveQuests() const
{
	TArray<UFVQuestInstance*> Result;
	for (const auto& Pair : ActiveQuests)
	{
		if (Pair.Value && Pair.Value->IsActive())
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

UFVQuestInstance* UFVQuestSubsystem::GetTrackedQuest() const
{
	return TrackedQuest.Get();
}

void UFVQuestSubsystem::SetTrackedQuest(UFVQuestInstance* Quest)
{
	if (Quest && Quest->QuestDefinition && Quest->QuestDefinition->bTrackable)
	{
		if (TrackedQuest.IsValid())
		{
			TrackedQuest->bIsTracked = false;
		}
		TrackedQuest = Quest;
		Quest->bIsTracked = true;
	}
	else
	{
		if (TrackedQuest.IsValid())
		{
			TrackedQuest->bIsTracked = false;
		}
		TrackedQuest.Reset();
	}
}

bool UFVQuestSubsystem::IsQuestAvailable(const UFVQuestDefinition* Definition) const
{
	if (!Definition)
	{
		return false;
	}

	const FFVQuestPrerequisite& Prerequisites = Definition->Prerequisites;
	
	UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>();
	if (!WorldState)
	{
		return false;
	}

	// Check required completed quests
	for (const FPrimaryAssetId& RequiredQuestId : Prerequisites.RequiredCompletedQuests)
	{
		// Extract quest name from asset ID
		FName QuestName = RequiredQuestId.PrimaryAssetName;
		if (!CompletedQuestIds.Contains(QuestName))
		{
			return false;
		}
	}

	// Check required world state tags
	if (!WorldState->HasAllWorldStateTags(Prerequisites.RequiredWorldStateTags))
	{
		return false;
	}

	// Check blocking world state tags
	if (WorldState->HasAnyWorldStateTags(Prerequisites.BlockingWorldStateTags))
	{
		return false;
	}

	// TODO: Check minimum sanity
	// TODO: Check clean state requirement
	// TODO: Check NPC trust levels
	// TODO: Check required memories

	return true;
}

TArray<UFVQuestDefinition*> UFVQuestSubsystem::GetAvailableQuests() const
{
	TArray<UFVQuestDefinition*> Result;
	
	// This would typically iterate through all registered quest definitions
	// For now, return empty array - implement based on your asset loading strategy
	
	return Result;
}

void UFVQuestSubsystem::RefreshQuestAvailability()
{
	// Iterate through all known quest definitions and check availability
	// Broadcast OnQuestAvailable for newly available quests
}

void UFVQuestSubsystem::NotifyTalkedToNPC(FName NPCId, FName DialogueNodeId)
{
	for (auto& Pair : ActiveQuests)
	{
		UFVQuestInstance* Quest = Pair.Value;
		if (!Quest || !Quest->IsActive())
		{
			continue;
		}

		for (const FName& ObjectiveId : Quest->GetActiveObjectiveIds())
		{
			// Find the objective definition
			if (Quest->QuestDefinition)
			{
				for (const UFVQuestObjective* Objective : Quest->QuestDefinition->Objectives)
				{
					if (const UFVObjective_TalkTo* TalkObjective = Cast<UFVObjective_TalkTo>(Objective))
					{
						if (TalkObjective->ObjectiveId == ObjectiveId && TalkObjective->TargetNPCId == NPCId)
						{
							FFVObjectiveProgress* Progress = Quest->GetObjectiveProgress(ObjectiveId);
							if (Progress)
							{
								Progress->CustomData.Add(FName("TalkedToNPC"), NPCId.ToString());
								if (DialogueNodeId != NAME_None)
								{
									Progress->CustomData.Add(FName("DialogueNodeReached"), DialogueNodeId.ToString());
								}

								if (TalkObjective->CheckCompletion(Quest))
								{
									Quest->CompleteObjective(ObjectiveId);
								}
							}
						}
					}
				}
			}
		}
	}
}

void UFVQuestSubsystem::NotifyItemObtained(FPrimaryAssetId ItemId, int32 Quantity)
{
	for (auto& Pair : ActiveQuests)
	{
		UFVQuestInstance* Quest = Pair.Value;
		if (!Quest || !Quest->IsActive())
		{
			continue;
		}

		for (const FName& ObjectiveId : Quest->GetActiveObjectiveIds())
		{
			if (Quest->QuestDefinition)
			{
				for (const UFVQuestObjective* Objective : Quest->QuestDefinition->Objectives)
				{
					if (const UFVObjective_ObtainItem* ItemObjective = Cast<UFVObjective_ObtainItem>(Objective))
					{
						if (ItemObjective->ObjectiveId == ObjectiveId && ItemObjective->ItemAssetId == ItemId)
						{
							Quest->UpdateObjectiveProgress(ObjectiveId, Quantity);
						}
					}
				}
			}
		}
	}
}

void UFVQuestSubsystem::NotifyLocationEntered(FName LocationId)
{
	for (auto& Pair : ActiveQuests)
	{
		UFVQuestInstance* Quest = Pair.Value;
		if (!Quest || !Quest->IsActive())
		{
			continue;
		}

		for (const FName& ObjectiveId : Quest->GetActiveObjectiveIds())
		{
			if (Quest->QuestDefinition)
			{
				for (const UFVQuestObjective* Objective : Quest->QuestDefinition->Objectives)
				{
					if (const UFVObjective_ReachLocation* LocationObjective = Cast<UFVObjective_ReachLocation>(Objective))
					{
						if (LocationObjective->ObjectiveId == ObjectiveId && LocationObjective->LocationId == LocationId)
						{
							FFVObjectiveProgress* Progress = Quest->GetObjectiveProgress(ObjectiveId);
							if (Progress)
							{
								Progress->CustomData.Add(FName("ReachedLocation"), LocationId.ToString());
								Quest->CompleteObjective(ObjectiveId);
							}
						}
					}
				}
			}
		}
	}
}

void UFVQuestSubsystem::NotifyThresholdReached(FGameplayTag ThresholdType, FName TargetId, float Value)
{
	for (auto& Pair : ActiveQuests)
	{
		UFVQuestInstance* Quest = Pair.Value;
		if (!Quest || !Quest->IsActive())
		{
			continue;
		}

		for (const FName& ObjectiveId : Quest->GetActiveObjectiveIds())
		{
			if (Quest->QuestDefinition)
			{
				for (const UFVQuestObjective* Objective : Quest->QuestDefinition->Objectives)
				{
					if (const UFVObjective_ReachThreshold* ThresholdObjective = Cast<UFVObjective_ReachThreshold>(Objective))
					{
						if (ThresholdObjective->ObjectiveId == ObjectiveId &&
							ThresholdObjective->ThresholdType.MatchesTagExact(ThresholdType))
						{
							FFVObjectiveProgress* Progress = Quest->GetObjectiveProgress(ObjectiveId);
							if (Progress)
							{
								Progress->CustomData.Add(FName("CurrentValue"), FString::SanitizeFloat(Value));
								
								if (ThresholdObjective->CheckCompletion(Quest))
								{
									Quest->CompleteObjective(ObjectiveId);
								}
							}
						}
					}
				}
			}
		}
	}
}

void UFVQuestSubsystem::NotifyTimeAdvanced(float GameHours)
{
	// Update wait-time and maintain-state objectives
	for (auto& Pair : ActiveQuests)
	{
		UFVQuestInstance* Quest = Pair.Value;
		if (!Quest || !Quest->IsActive())
		{
			continue;
		}

		for (const FName& ObjectiveId : Quest->GetActiveObjectiveIds())
		{
			FFVObjectiveProgress* Progress = Quest->GetObjectiveProgress(ObjectiveId);
			if (!Progress)
			{
				continue;
			}

			if (Quest->QuestDefinition)
			{
				for (const UFVQuestObjective* Objective : Quest->QuestDefinition->Objectives)
				{
					if (Objective->ObjectiveId != ObjectiveId)
					{
						continue;
					}

					if (const UFVObjective_WaitTime* WaitObjective = Cast<UFVObjective_WaitTime>(Objective))
					{
						const FString* ElapsedStr = Progress->CustomData.Find(FName("ElapsedHours"));
						float Elapsed = ElapsedStr ? FCString::Atof(**ElapsedStr) : 0.0f;
						Elapsed += GameHours;
						Progress->CustomData.Add(FName("ElapsedHours"), FString::SanitizeFloat(Elapsed));

						if (WaitObjective->CheckCompletion(Quest))
						{
							Quest->CompleteObjective(ObjectiveId);
						}
					}
					else if (const UFVObjective_MaintainState* MaintainObjective = Cast<UFVObjective_MaintainState>(Objective))
					{
						// Check if state is still valid
						// TODO: Check tag presence via WorldState
						const FString* MaintainedStr = Progress->CustomData.Find(FName("MaintainedHours"));
						float Maintained = MaintainedStr ? FCString::Atof(**MaintainedStr) : 0.0f;
						Maintained += GameHours;
						Progress->CustomData.Add(FName("MaintainedHours"), FString::SanitizeFloat(Maintained));

						if (MaintainObjective->CheckCompletion(Quest))
						{
							Quest->CompleteObjective(ObjectiveId);
						}
					}
				}
			}
		}

		// Check time limit
		if (Quest->QuestDefinition && Quest->QuestDefinition->TimeLimitHours > 0.0f)
		{
			UFVWorldStateSubsystem* WorldState = GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>();
			if (WorldState)
			{
				const float ElapsedTime = WorldState->GetCurrentGameTime() - Quest->TimeStarted;
				if (ElapsedTime >= Quest->QuestDefinition->TimeLimitHours)
				{
					Quest->FailQuest();
				}
			}
		}
	}
}

bool UFVQuestSubsystem::HasCompletedQuest(FName QuestId) const
{
	return CompletedQuestIds.Contains(QuestId);
}

bool UFVQuestSubsystem::HasFailedQuest(FName QuestId) const
{
	return FailedQuestIds.Contains(QuestId);
}

TArray<FName> UFVQuestSubsystem::GetCompletedQuestIds() const
{
	return CompletedQuestIds.Array();
}

TArray<uint8> UFVQuestSubsystem::SerializeQuestData() const
{
	// TODO: Implement proper serialization
	TArray<uint8> Data;
	return Data;
}

void UFVQuestSubsystem::DeserializeQuestData(const TArray<uint8>& Data)
{
	// TODO: Implement proper deserialization
}

void UFVQuestSubsystem::OnQuestStateChanged(UFVQuestInstance* Quest, FGameplayTag NewState)
{
	if (!Quest || !Quest->QuestDefinition)
	{
		return;
	}

	const FName QuestId = Quest->QuestDefinition->QuestId;

	if (NewState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Completed"))))
	{
		CompletedQuestIds.Add(QuestId);
		ActiveQuests.Remove(QuestId);
		
		if (TrackedQuest == Quest)
		{
			SetTrackedQuest(nullptr);
		}

		OnQuestCompleted.Broadcast(Quest);
		RefreshQuestAvailability();
	}
	else if (NewState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Failed"))) ||
			 NewState.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Quest.State.Abandoned"))))
	{
		FailedQuestIds.Add(QuestId);
		ActiveQuests.Remove(QuestId);

		if (TrackedQuest == Quest)
		{
			SetTrackedQuest(nullptr);
		}

		OnQuestFailed.Broadcast(Quest);
		RefreshQuestAvailability();
	}
}