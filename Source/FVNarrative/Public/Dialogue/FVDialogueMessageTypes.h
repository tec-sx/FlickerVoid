#pragma once

#include "CoreMinimal.h"
#include "FVDialogueMessageTypes.generated.h"

struct FSUDSValue;


USTRUCT(BlueprintType)
struct FFVDialogueLineMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName SpeakerID;
	UPROPERTY(BlueprintReadOnly) FText SpeakerDisplayName;
	UPROPERTY(BlueprintReadOnly) FText Text;
	UPROPERTY(BlueprintReadOnly) bool bIsVoiced = false;
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> OwnerActor;
};

USTRUCT(BlueprintType)
struct FFVDialogueChoiceMessage
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
struct FFVDialogueChoicesMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) TArray<FFVDialogueChoiceMessage> Choices;
};

USTRUCT(BlueprintType)
struct FFVDialogueEventMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName EventName;
	UPROPERTY(BlueprintReadOnly) TArray<FString> Arguments;
};

USTRUCT(BlueprintType)
struct FFVDialogueSubmitChoiceMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) int32 ChoiceIndex = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite) FName ChoiceId;	// preferred when using locked previews
};

USTRUCT(BlueprintType)
struct FFVDialogueEndedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bWasAborted = false;
	UPROPERTY(BlueprintReadOnly) bool bStartedContextAction = false;
	UPROPERTY(BlueprintReadOnly) FName ContextActionId;
};

USTRUCT(BlueprintType)
struct FFVDialogueInteractMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) TWeakObjectPtr<AActor> Interactor;
	UPROPERTY(BlueprintReadWrite) TWeakObjectPtr<AActor> TargetNPC;
};

USTRUCT(BlueprintType)
struct FFVDialogueSightLostMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) TWeakObjectPtr<AActor> NPC;
};

USTRUCT(BlueprintType)
struct FFVDialogueCallOutMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName SpeakerID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TWeakObjectPtr<AActor> OwnerActor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> Voice;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DisplayDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bShowTalkIcon = true;
};