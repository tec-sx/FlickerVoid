// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest/FVQuestDefinition.h"
#include "Quest/FVQuestObjective.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestDefinition)

UFVQuestDefinition::UFVQuestDefinition()
{
	QuestId = NAME_None;
	CompletionType = EFVQuestCompletionType::AllObjectives;
	MinimumObjectivesRequired = 1;
	bTrackable = true;
	bCanAbandon = true;
	TimeLimitHours = 0.0f;
	bRepeatable = false;
	Priority = 0;
}

FPrimaryAssetId UFVQuestDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("FVQuest"), GetFName());
}