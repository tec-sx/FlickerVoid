// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVUIDialogueMessageTypes.generated.h"

USTRUCT(BlueprintType)
struct FFVUIDialogueLineMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName SpeakerID;
	UPROPERTY(BlueprintReadOnly) FText SpeakerDisplayName;
	UPROPERTY(BlueprintReadOnly) FText Text;
	UPROPERTY(BlueprintReadOnly) bool bIsVoiced = false;
};

USTRUCT(BlueprintType)
struct FFVUIDialogueChoiceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 Index = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly) FName ChoiceId;
	UPROPERTY(BlueprintReadOnly) FText Text;
	UPROPERTY(BlueprintReadOnly) FText RequirementTooltip;
	UPROPERTY(BlueprintReadOnly) bool bWasTakenBefore = false;
	UPROPERTY(BlueprintReadOnly) bool bIsEnabled = true;
};

USTRUCT(BlueprintType)
struct FFVUIDialogueChoicesMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) TArray<FFVUIDialogueChoiceMessage> Choices;
};

USTRUCT(BlueprintType)
struct FFVUIDialogueEventMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName EventName;
	UPROPERTY(BlueprintReadOnly) TArray<FString> Arguments;
};

USTRUCT(BlueprintType)
struct FFVUIDialogueSubmitChoiceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) int32 ChoiceIndex = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite) FName ChoiceId;
};

USTRUCT(BlueprintType)
struct FFVUIDialogueEndedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bWasAborted = false;
	UPROPERTY(BlueprintReadOnly) bool bStartedContextAction = false;
	UPROPERTY(BlueprintReadOnly) FName ContextActionId;
};
