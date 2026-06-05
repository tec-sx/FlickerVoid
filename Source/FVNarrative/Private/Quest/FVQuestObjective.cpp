// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest/FVQuestObjective.h"
#include "Quest/FVQuestInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestObjective)

// ============================================================================
// BASE OBJECTIVE
// ============================================================================

UFVQuestObjective::UFVQuestObjective()
{
	ObjectiveId = NAME_None;
	TargetCount = 1;
	bOptional = false;
	bHiddenUntilActive = false;
}

bool UFVQuestObjective::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	// Base implementation - override in subclasses
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	return Progress && Progress->CurrentCount >= TargetCount;
}

float UFVQuestObjective::GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance || TargetCount <= 0)
	{
		return 0.0f;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(Progress->CurrentCount) / static_cast<float>(TargetCount), 0.0f, 1.0f);
}

FText UFVQuestObjective::GetProgressText_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return FText::GetEmpty();
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return FText::GetEmpty();
	}

	if (TargetCount > 1)
	{
		return FText::Format(NSLOCTEXT("QuestObjective", "ProgressFormat", "{0} / {1}"),
			FText::AsNumber(Progress->CurrentCount),
			FText::AsNumber(TargetCount));
	}

	return FText::GetEmpty();
}

// ============================================================================
// TALK TO NPC OBJECTIVE
// ============================================================================

UFVObjective_TalkTo::UFVObjective_TalkTo()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.Talk"));
	TargetNPCId = NAME_None;
	RequiredDialogueNodeId = NAME_None;
}

bool UFVObjective_TalkTo::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return false;
	}

	// Check if the required NPC was talked to
	const FString* TalkedTo = Progress->CustomData.Find(FName("TalkedToNPC"));
	if (!TalkedTo || *TalkedTo != TargetNPCId.ToString())
	{
		return false;
	}

	// If a specific dialogue node is required, check that too
	if (RequiredDialogueNodeId != NAME_None)
	{
		const FString* DialogueNode = Progress->CustomData.Find(FName("DialogueNodeReached"));
		if (!DialogueNode || *DialogueNode != RequiredDialogueNodeId.ToString())
		{
			return false;
		}
	}

	return true;
}

// ============================================================================
// OBTAIN ITEM OBJECTIVE
// ============================================================================

UFVObjective_ObtainItem::UFVObjective_ObtainItem()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.Obtain"));
	Quantity = 1;
}

bool UFVObjective_ObtainItem::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	return Progress && Progress->CurrentCount >= Quantity;
}

float UFVObjective_ObtainItem::GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance || Quantity <= 0)
	{
		return 0.0f;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(Progress->CurrentCount) / static_cast<float>(Quantity), 0.0f, 1.0f);
}

// ============================================================================
// REACH LOCATION OBJECTIVE
// ============================================================================

UFVObjective_ReachLocation::UFVObjective_ReachLocation()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.Location"));
	LocationId = NAME_None;
}

bool UFVObjective_ReachLocation::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return false;
	}

	const FString* ReachedLocation = Progress->CustomData.Find(FName("ReachedLocation"));
	return ReachedLocation && *ReachedLocation == LocationId.ToString();
}

// ============================================================================
// REACH THRESHOLD OBJECTIVE
// ============================================================================

UFVObjective_ReachThreshold::UFVObjective_ReachThreshold()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.Threshold"));
	TargetNPCId = NAME_None;
	ThresholdValue = 0.5f;
	bGreaterThanOrEqual = true;
}

bool UFVObjective_ReachThreshold::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return false;
	}

	const FString* CurrentValueStr = Progress->CustomData.Find(FName("CurrentValue"));
	if (!CurrentValueStr)
	{
		return false;
	}

	const float CurrentValue = FCString::Atof(**CurrentValueStr);
	
	if (bGreaterThanOrEqual)
	{
		return CurrentValue >= ThresholdValue;
	}
	else
	{
		return CurrentValue <= ThresholdValue;
	}
}

float UFVObjective_ReachThreshold::GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return 0.0f;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return 0.0f;
	}

	const FString* CurrentValueStr = Progress->CustomData.Find(FName("CurrentValue"));
	if (!CurrentValueStr)
	{
		return 0.0f;
	}

	const float CurrentValue = FCString::Atof(**CurrentValueStr);
	
	if (bGreaterThanOrEqual)
	{
		return FMath::Clamp(CurrentValue / ThresholdValue, 0.0f, 1.0f);
	}
	else
	{
		// Inverse for "less than" thresholds
		if (ThresholdValue <= 0.0f)
		{
			return 1.0f;
		}
		return FMath::Clamp(1.0f - (CurrentValue / ThresholdValue), 0.0f, 1.0f);
	}
}

// ============================================================================
// WAIT TIME OBJECTIVE
// ============================================================================

UFVObjective_WaitTime::UFVObjective_WaitTime()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.Wait"));
	WaitHours = 1.0f;
}

bool UFVObjective_WaitTime::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return false;
	}

	const FString* ElapsedStr = Progress->CustomData.Find(FName("ElapsedHours"));
	if (!ElapsedStr)
	{
		return false;
	}

	const float ElapsedHours = FCString::Atof(**ElapsedStr);
	return ElapsedHours >= WaitHours;
}

float UFVObjective_WaitTime::GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance || WaitHours <= 0.0f)
	{
		return 0.0f;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return 0.0f;
	}

	const FString* ElapsedStr = Progress->CustomData.Find(FName("ElapsedHours"));
	if (!ElapsedStr)
	{
		return 0.0f;
	}

	const float ElapsedHours = FCString::Atof(**ElapsedStr);
	return FMath::Clamp(ElapsedHours / WaitHours, 0.0f, 1.0f);
}

// ============================================================================
// MAINTAIN STATE OBJECTIVE
// ============================================================================

UFVObjective_MaintainState::UFVObjective_MaintainState()
{
	ObjectiveType = FGameplayTag::RequestGameplayTag(FName("Objective.Type.MaintainState"));
	bRequireTagPresent = true;
	DurationHours = 24.0f;
}

bool UFVObjective_MaintainState::CheckCompletion_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance)
	{
		return false;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return false;
	}

	const FString* MaintainedHoursStr = Progress->CustomData.Find(FName("MaintainedHours"));
	if (!MaintainedHoursStr)
	{
		return false;
	}

	const float MaintainedHours = FCString::Atof(**MaintainedHoursStr);
	return MaintainedHours >= DurationHours;
}

float UFVObjective_MaintainState::GetProgress_Implementation(const UFVQuestInstance* QuestInstance) const
{
	if (!QuestInstance || DurationHours <= 0.0f)
	{
		return 0.0f;
	}

	const FFVObjectiveProgress* Progress = const_cast<UFVQuestInstance*>(QuestInstance)->GetObjectiveProgress(ObjectiveId);
	if (!Progress)
	{
		return 0.0f;
	}

	const FString* MaintainedHoursStr = Progress->CustomData.Find(FName("MaintainedHours"));
	if (!MaintainedHoursStr)
	{
		return 0.0f;
	}

	const float MaintainedHours = FCString::Atof(**MaintainedHoursStr);
	return FMath::Clamp(MaintainedHours / DurationHours, 0.0f, 1.0f);
}