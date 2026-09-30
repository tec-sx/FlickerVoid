#include "Dialogue/FVDialogueSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Yap/Interfaces/IYapCharacterInterface.h"
#include "Yap/YapCharacterComponent.h"
#include "Yap/YapConversation.h"
#include "Yap/YapSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueSubsystem)

UFVDialogueSubsystem* UFVDialogueSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVDialogueSubsystem>() : nullptr;
}

AActor* UFVDialogueSubsystem::FindSpeakerActor(const UObject* WorldContext, FName SpeakerID)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || SpeakerID.IsNone())
	{
		return nullptr;
	}
	const UYapCharacterComponent* Component = UYapSubsystem::FindCharacterComponent(World, SpeakerID);
	return Component ? Component->GetOwner() : nullptr;
}

bool UFVDialogueSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVDialogueSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	UYapSubsystem::RegisterConversationHandler(this, FYapDialogueNodeClassType());
	bRegistered = true;
}

void UFVDialogueSubsystem::Deinitialize()
{
	if (bRegistered)
	{
		UYapSubsystem::UnregisterConversationHandler(this, FYapDialogueNodeClassType());
		bRegistered = false;
	}
	Super::Deinitialize();
}

void UFVDialogueSubsystem::OnConversationOpened(FYapData_ConversationOpened Data, FYapConversationHandle Handle)
{
	Conversation = Handle;
	CurrentLine = FFVDialogueLine();
	Prompts.Reset();

	if (FYapConversation* Conv = UYapSubsystem::GetConversationByHandle(this, Handle))
	{
		Conv->OnConversationClosed.AddUniqueDynamic(this, &UFVDialogueSubsystem::HandleConversationClosed);
	}
	OnConversationStarted.Broadcast();
}

void UFVDialogueSubsystem::OnConversationSpeechBegins(FYapData_SpeechBegins Data, FYapSpeechHandle Handle)
{
	Speech = Handle;
	Prompts.Reset();

	CurrentLine.SpeakerID = Data.SpeakerID;
	CurrentLine.SpeakerName = Data.Speaker ? Data.Speaker->GetCharacterName() : FText::FromName(Data.SpeakerID);
	CurrentLine.SpeakerActor = FindSpeakerActor(this, Data.SpeakerID);
	CurrentLine.Mood = Data.MoodTag;
	CurrentLine.Text = Data.DialogueText;
	CurrentLine.Title = Data.TitleText;
	CurrentLine.Duration = Data.SpeechTime;
	CurrentLine.bSkippable = Data.bSkippable;
	OnLineChanged.Broadcast();
}

void UFVDialogueSubsystem::OnConversationPlayerPromptCreated(FYapData_PlayerPromptCreated Data, FYapPromptHandle Handle)
{
	FFVDialoguePrompt& Prompt = Prompts.AddDefaulted_GetRef();
	Prompt.Text = Data.DialogueText;
	Prompt.Title = Data.TitleText;
	Prompt.Handle = Handle;
}

void UFVDialogueSubsystem::OnConversationPlayerPromptsReady(FYapData_PlayerPromptsReady Data)
{
	OnPromptsReady.Broadcast();
}

void UFVDialogueSubsystem::OnConversationPlayerPromptChosen(FYapData_PlayerPromptChosen Data, FYapPromptHandle Handle)
{
	Prompts.Reset();
}

void UFVDialogueSubsystem::ChoosePrompt(int32 Index)
{
	if (!Prompts.IsValidIndex(Index))
	{
		return;
	}
	const FYapPromptHandle Handle = Prompts[Index].Handle;
	UYapSubsystem::RunPrompt(this, Handle);
}

void UFVDialogueSubsystem::Advance()
{
	if (Speech.IsValid())
	{
		UYapSubsystem::AdvanceSpeech(this, Speech);
	}
}

void UFVDialogueSubsystem::HandleConversationClosed(UObject* Instigator, FYapConversationHandle Handle)
{
	if (Handle != Conversation)
	{
		return;
	}
	Conversation = FYapConversationHandle();
	Speech = FYapSpeechHandle();
	CurrentLine = FFVDialogueLine();
	Prompts.Reset();
	OnConversationEnded.Broadcast();
}

AActor* UFVDialogueSubsystem::FindSpeakerActorForContext(const UObject* WorldContext)
{
	const UFVDialogueSubsystem* Subsystem = Get(WorldContext);
	return Subsystem ? Subsystem->GetCurrentSpeaker() : nullptr;
}
