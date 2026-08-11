// Fill out your copyright notice in the Description page of Project Settings.


#include "FVDialogueUIManagerBase.h"

#include "FVUITags.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/FVUIDialogueMessageTypes.h"

UFVDialogueUIManagerBase* UFVDialogueUIManagerBase::Get(UObject* WorldContextObject)
{
	APlayerController* PC = Cast<APlayerController>(WorldContextObject);
	if (!PC)
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			PC = World->GetFirstPlayerController();
		}
	}
	
	if (!PC)
	{
		return nullptr;
	}
	
	return NewObject<UFVDialogueUIManagerBase>(PC);
}

void UFVDialogueUIManagerBase::Initialize(TSubclassOf<UUserWidget> DialogueWidgetClass, APlayerController* InPC)
{
	if (bIsInitialized)
	{
		return;
	}
	
	OwningPC = InPC;
	RegisterListeners();
	bIsInitialized = true;
	
	OnManagerInitialized(DialogueWidgetClass, InPC);
}

void UFVDialogueUIManagerBase::Deinitialize()
{
	if (!bIsInitialized) return;

	UnregisterListeners();
	OnManagerDeinitialized();

	OwningPC.Reset();
	bIsInitialized = false;
}

void UFVDialogueUIManagerBase::RegisterListeners()
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	LineReadyHandle = Router.RegisterListener<FFVUIDialogueLineMessage>(FVUITags::Dialogue_LineReady, this, &UFVDialogueUIManagerBase::HandleLineReady);
	ChoicesReadyHandle = Router.RegisterListener<FFVUIDialogueChoicesMessage>(FVUITags::Dialogue_ChoicesReady, this, &UFVDialogueUIManagerBase::HandleChoicesReady);
	EventHandle = Router.RegisterListener<FFVUIDialogueEventMessage>(FVUITags::Dialogue_Event, this, &UFVDialogueUIManagerBase::HandleEvent);
	EndedHandle = Router.RegisterListener<FFVUIDialogueEndedMessage>(FVUITags::Dialogue_Ended, this, &UFVDialogueUIManagerBase::HandleEnded);
}

void UFVDialogueUIManagerBase::UnregisterListeners()
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	if (LineReadyHandle.IsValid())     Router.UnregisterListener(LineReadyHandle);
	if (ChoicesReadyHandle.IsValid())  Router.UnregisterListener(ChoicesReadyHandle);
	if (EventHandle.IsValid())         Router.UnregisterListener(EventHandle);
	if (EndedHandle.IsValid())         Router.UnregisterListener(EndedHandle);
	
	LineReadyHandle = ChoicesReadyHandle = EventHandle = EndedHandle = {};
}

void UFVDialogueUIManagerBase::SubmitChoice(int32 ChoiceIndex)
{
	FFVUIDialogueSubmitChoiceMessage Msg;
	Msg.ChoiceIndex = ChoiceIndex;

	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVUITags::Dialogue_SubmitChoice, Msg);
}

void UFVDialogueUIManagerBase::SubmitContinue()
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVUITags::Dialogue_Continue, FFVUIDialogueEndedMessage{});
}

void UFVDialogueUIManagerBase::HandleLineReady(FGameplayTag Channel, const FFVUIDialogueLineMessage& Message)
{
	OnDialogueLineReady(Message);
}

void UFVDialogueUIManagerBase::HandleChoicesReady(FGameplayTag Channel, const FFVUIDialogueChoicesMessage& Message)
{
	OnDialogueChoicesReady(Message);
}

void UFVDialogueUIManagerBase::HandleEvent(FGameplayTag Channel, const FFVUIDialogueEventMessage& Message)
{
	OnDialogueEvent(Message);
}

void UFVDialogueUIManagerBase::HandleEnded(FGameplayTag Channel, const FFVUIDialogueEndedMessage& Message)
{
	OnDialogueEnded(Message);
}
