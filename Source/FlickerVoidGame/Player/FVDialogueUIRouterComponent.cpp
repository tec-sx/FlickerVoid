// Fill out your copyright notice in the Description page of Project Settings.

#include "FVDialogueUIRouterComponent.h"

#include "FVNarrativeTags.h"
#include "FVUITags.h"
#include "Dialogue/FVDialogueMessageTypes.h"
#include "Dialogue/FVDialogueMessageLibrary.h"
#include "Dialogue/FVUIDialogueMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueUIRouterComponent)

UFVDialogueUIRouterComponent::UFVDialogueUIRouterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }

void UFVDialogueUIRouterComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	LineReadyHandle = Router.RegisterListener(FVNarrativeTags::Dialogue_LineReady, this, &UFVDialogueUIRouterComponent::HandleLineReady);
	ChoicesReadyHandle = Router.RegisterListener(FVNarrativeTags::Dialogue_ChoicesReady, this, &UFVDialogueUIRouterComponent::HandleChoicesReady);
	EventHandle = Router.RegisterListener(FVNarrativeTags::Dialogue_Event, this, &UFVDialogueUIRouterComponent::HandleEvent);
	EndedHandle = Router.RegisterListener(FVNarrativeTags::Dialogue_Ended, this, &UFVDialogueUIRouterComponent::HandleEnded);

	SubmitChoiceHandle = Router.RegisterListener(FVUITags::Dialogue_SubmitChoice, this, &UFVDialogueUIRouterComponent::HandleUISubmitChoice);
	ContinueHandle = Router.RegisterListener(FVUITags::Dialogue_Continue, this, &UFVDialogueUIRouterComponent::HandleUIContinue);
}

void UFVDialogueUIRouterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	if (LineReadyHandle.IsValid())     Router.UnregisterListener(LineReadyHandle);
	if (ChoicesReadyHandle.IsValid())  Router.UnregisterListener(ChoicesReadyHandle);
	if (EventHandle.IsValid())         Router.UnregisterListener(EventHandle);
	if (EndedHandle.IsValid())         Router.UnregisterListener(EndedHandle);
	if (SubmitChoiceHandle.IsValid())  Router.UnregisterListener(SubmitChoiceHandle);
	if (ContinueHandle.IsValid())      Router.UnregisterListener(ContinueHandle);

	Super::EndPlay(EndPlayReason);
}

void UFVDialogueUIRouterComponent::HandleLineReady(FGameplayTag Channel, const FFVDialogueLineMessage& Message)
{
	FFVUIDialogueLineMessage UIMessage;
	UIMessage.SpeakerID = Message.SpeakerID;
	UIMessage.SpeakerDisplayName = Message.SpeakerDisplayName;
	UIMessage.Text = Message.Text;
	UIMessage.bIsVoiced = Message.bIsVoiced;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVUITags::Dialogue_LineReady, UIMessage);
}

void UFVDialogueUIRouterComponent::HandleChoicesReady(FGameplayTag Channel, const FFVDialogueChoicesMessage& Message)
{
	FFVUIDialogueChoicesMessage UIMessage;
	UIMessage.Choices.Reserve(Message.Choices.Num());

	for (const FFVDialogueChoiceMessage& Choice : Message.Choices)
	{
		FFVUIDialogueChoiceMessage UIChoice;
		UIChoice.Index = Choice.Index;
		UIChoice.ChoiceId = Choice.ChoiceId;
		UIChoice.Text = Choice.Text;
		UIChoice.RequirementTooltip = Choice.RequirementTooltip;
		UIChoice.bWasTakenBefore = Choice.bWasTakenBefore;
		UIChoice.bIsEnabled = Choice.bIsEnabled;

		UIMessage.Choices.Add(UIChoice);
	}

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVUITags::Dialogue_ChoicesReady, UIMessage);
}

void UFVDialogueUIRouterComponent::HandleEvent(FGameplayTag Channel, const FFVDialogueEventMessage& Message)
{
	FFVUIDialogueEventMessage UIMessage;
	UIMessage.EventName = Message.EventName;
	UIMessage.Arguments = Message.Arguments;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVUITags::Dialogue_Event, UIMessage);
}

void UFVDialogueUIRouterComponent::HandleEnded(FGameplayTag Channel, const FFVDialogueEndedMessage& Message)
{
	FFVUIDialogueEndedMessage UIMessage;
	UIMessage.bWasAborted = Message.bWasAborted;
	UIMessage.bStartedContextAction = Message.bStartedContextAction;
	UIMessage.ContextActionId = Message.ContextActionId;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(FVUITags::Dialogue_Ended, UIMessage);
}

void UFVDialogueUIRouterComponent::HandleUISubmitChoice(FGameplayTag Channel, const FFVUIDialogueSubmitChoiceMessage& Message)
{
	FFVDialogueSubmitChoiceMessage NarrativeMessage;
	NarrativeMessage.ChoiceIndex = Message.ChoiceIndex;
	NarrativeMessage.ChoiceId = Message.ChoiceId;

	UFVDialogueMessageLibrary::BroadcastDialogueSubmitChoice(this, NarrativeMessage);
}

void UFVDialogueUIRouterComponent::HandleUIContinue(FGameplayTag Channel, const FFVUIDialogueEndedMessage& Message)
{
	FFVDialogueEndedMessage NarrativeMessage;
	NarrativeMessage.bWasAborted = Message.bWasAborted;
	NarrativeMessage.bStartedContextAction = Message.bStartedContextAction;
	NarrativeMessage.ContextActionId = Message.ContextActionId;

	UFVDialogueMessageLibrary::BroadcastDialogueContinue(this, NarrativeMessage);
}
