// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "FVDialogueUIRouterComponent.generated.h"

struct FFVDialogueLineMessage;
struct FFVDialogueChoicesMessage;
struct FFVDialogueEventMessage;
struct FFVDialogueEndedMessage;
struct FFVUIDialogueSubmitChoiceMessage;
struct FFVUIDialogueEndedMessage;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLICKERVOID_API UFVDialogueUIRouterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVDialogueUIRouterComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleLineReady(FGameplayTag Channel, const FFVDialogueLineMessage& Message);
	void HandleChoicesReady(FGameplayTag Channel, const FFVDialogueChoicesMessage& Message);
	void HandleEvent(FGameplayTag Channel, const FFVDialogueEventMessage& Message);
	void HandleEnded(FGameplayTag Channel, const FFVDialogueEndedMessage& Message);

	void HandleUISubmitChoice(FGameplayTag Channel, const FFVUIDialogueSubmitChoiceMessage& Message);
	void HandleUIContinue(FGameplayTag Channel, const FFVUIDialogueEndedMessage& Message);

	FGameplayMessageListenerHandle LineReadyHandle;
	FGameplayMessageListenerHandle ChoicesReadyHandle;
	FGameplayMessageListenerHandle EventHandle;
	FGameplayMessageListenerHandle EndedHandle;
	FGameplayMessageListenerHandle SubmitChoiceHandle;
	FGameplayMessageListenerHandle ContinueHandle;
};
