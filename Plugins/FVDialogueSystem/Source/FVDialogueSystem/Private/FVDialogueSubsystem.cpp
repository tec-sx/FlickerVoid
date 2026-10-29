#include "FVDialogueSubsystem.h"

#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FlowAsset.h"
#include "FlowSubsystem.h"
#include "Interfaces/FlowDataPinValueSupplierInterface.h"
#include "FVDialogueParticipantComponent.h"
#include "FVDialogueSystem.h"
#include "Identity/FVIdentityComponent.h"
#include "Identity/FVIdentitySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueSubsystem)

UFVDialogueSubsystem* UFVDialogueSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVDialogueSubsystem>() : nullptr;
}

bool UFVDialogueSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVDialogueSubsystem::Deinitialize()
{
	CancelPending();
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AbortTimer);
		World->GetTimerManager().ClearTimer(CheckTimer);
	}
	Super::Deinitialize();
}

bool UFVDialogueSubsystem::StartConversation(UFlowAsset* InDialogue, AActor* InInstigator, AActor* InOwner)
{
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UFlowSubsystem* FlowSubsystem = GameInstance ? GameInstance->GetSubsystem<UFlowSubsystem>() : nullptr;
	if (bActive || !InDialogue || !InOwner || !FlowSubsystem)
	{
		UE_LOG(LogFVDialogueSystem, Warning, TEXT("Can't start conversation %s with %s."), *GetNameSafe(InDialogue), *GetNameSafe(InOwner));
		return false;
	}

	// A conversation ended this frame still has its flow instance, which would block the new one.
	FlushPendingAbort();

	UFlowAsset* Instance = FlowSubsystem->CreateRootFlow(InOwner, InDialogue, false);
	if (!Instance)
	{
		UE_LOG(LogFVDialogueSystem, Warning, TEXT("Conversation %s with %s is already running."), *GetNameSafe(InDialogue), *GetNameSafe(InOwner));
		return false;
	}

	BeginConversation(InInstigator, InOwner, Instance);
	Dialogue = InDialogue;
	Instance->StartFlow();
	return true;
}

void UFVDialogueSubsystem::BeginConversation(AActor* InInstigator, AActor* InOwner, UFlowAsset* InFlow)
{
	if (bActive)
	{
		return;
	}

	bActive = true;
	Instigator = InInstigator ? InInstigator : UGameplayStatics::GetPlayerPawn(this, 0);
	Owner = InOwner;
	Flow = InFlow;
	GetWorld()->GetTimerManager().SetTimer(CheckTimer, this, &UFVDialogueSubsystem::CheckConversation, 0.25f, true);
	OnConversationStarted.Broadcast();
}

void UFVDialogueSubsystem::CheckConversation()
{
	const bool bFlowStopped = Flow.IsExplicitlyNull() ? false : !Flow.IsValid() || !Flow->IsActive();
	if (bActive && (bFlowStopped || !Owner.IsValid()))
	{
		UE_LOG(LogFVDialogueSystem, Log, TEXT("Closing conversation %s: %s."), *GetNameSafe(Dialogue), bFlowStopped ? TEXT("its flow stopped") : TEXT("its owner is gone"));
		Close(!bFlowStopped);
	}
}

void UFVDialogueSubsystem::EndConversation()
{
	Close(true);
}

void UFVDialogueSubsystem::EndConversationFromFlow()
{
	Close(false);
}

void UFVDialogueSubsystem::Close(const bool bAbortFlow)
{
	if (!bActive)
	{
		return;
	}

	CancelPending();
	GetWorld()->GetTimerManager().ClearTimer(CheckTimer);

	AActor* FlowOwner = Owner.Get();
	UFlowAsset* FlowTemplate = Dialogue;

	bActive = false;
	Instigator.Reset();
	Owner.Reset();
	Flow.Reset();
	Dialogue = nullptr;
	CurrentLine = FFVDialogueLine();
	Choices.Reset();

	// Deferred so it's safe to end from inside a running node; a new conversation flushes it first.
	if (bAbortFlow && FlowOwner && FlowTemplate)
	{
		FlushPendingAbort();
		PendingAbortOwner = FlowOwner;
		PendingAbortTemplate = FlowTemplate;
		GetWorld()->GetTimerManager().SetTimer(AbortTimer, this, &UFVDialogueSubsystem::FlushPendingAbort, KINDA_SMALL_NUMBER, false);
	}

	OnConversationEnded.Broadcast();
}

void UFVDialogueSubsystem::FlushPendingAbort()
{
	GetWorld()->GetTimerManager().ClearTimer(AbortTimer);

	AActor* AbortOwner = PendingAbortOwner.Get();
	UFlowAsset* AbortTemplate = PendingAbortTemplate.Get();
	PendingAbortOwner.Reset();
	PendingAbortTemplate.Reset();

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UFlowSubsystem* FlowSubsystem = GameInstance ? GameInstance->GetSubsystem<UFlowSubsystem>() : nullptr;
	if (FlowSubsystem && AbortOwner && AbortTemplate)
	{
		FlowSubsystem->FinishRootFlow(AbortOwner, AbortTemplate, EFlowFinishPolicy::Abort);
	}
}

AActor* UFVDialogueSubsystem::ResolveSpeaker(const UFVCharacterDefinition* Speaker) const
{
	if (!Speaker)
	{
		return Owner.Get();
	}

	for (AActor* Candidate : { Instigator.Get(), Owner.Get() })
	{
		if (Candidate && UFVIdentityComponent::GetActorDefinition(Candidate) == Speaker)
		{
			return Candidate;
		}
	}

	const UFVIdentitySubsystem* Identities = UFVIdentitySubsystem::Get(this);
	return Identities ? Identities->FindActor(Speaker) : nullptr;
}

void UFVDialogueSubsystem::PlayLine(const FFVDialogueLine& Line, FFVDialogueLineFinished OnFinished)
{
	CancelPending();

	CurrentLine = Line;
	CurrentLine.Duration = ComputeDuration(Line);
	LineFinished = MoveTemp(OnFinished);

	if (CurrentLine.Voice && CurrentLine.SpeakerActor)
	{
		Voice = UGameplayStatics::SpawnSoundAttached(CurrentLine.Voice, CurrentLine.SpeakerActor->GetRootComponent());
	}
	else if (CurrentLine.Voice)
	{
		Voice = UGameplayStatics::SpawnSound2D(this, CurrentLine.Voice);
	}

	if (!CurrentLine.bWaitForInput)
	{
		GetWorld()->GetTimerManager().SetTimer(LineTimer, this, &UFVDialogueSubsystem::FinishLine, FMath::Max(CurrentLine.Duration, KINDA_SMALL_NUMBER), false);
	}

	if (UFVDialogueParticipantComponent* Participant = UFVDialogueParticipantComponent::Find(CurrentLine.SpeakerActor))
	{
		Participant->OnLineStarted.Broadcast(CurrentLine);
	}
	OnLineStarted.Broadcast(CurrentLine);
}

void UFVDialogueSubsystem::Advance()
{
	if (LineFinished.IsBound() && (CurrentLine.bSkippable || CurrentLine.bWaitForInput))
	{
		FinishLine();
	}
}

void UFVDialogueSubsystem::FinishLine()
{
	GetWorld()->GetTimerManager().ClearTimer(LineTimer);
	StopVoice();

	const FFVDialogueLine Finished = CurrentLine;
	FFVDialogueLineFinished Callback = MoveTemp(LineFinished);
	LineFinished.Unbind();

	if (UFVDialogueParticipantComponent* Participant = UFVDialogueParticipantComponent::Find(Finished.SpeakerActor))
	{
		Participant->OnLineFinished.Broadcast(Finished);
	}
	OnLineFinished.Broadcast(Finished);

	Callback.ExecuteIfBound();
}

void UFVDialogueSubsystem::PresentChoices(const TArray<FFVDialogueChoice>& InChoices, FFVDialogueChoiceMade OnChosen)
{
	CancelPending();

	Choices = InChoices;
	ChoiceMade = MoveTemp(OnChosen);
	OnChoicesPresented.Broadcast(Choices);
}

bool UFVDialogueSubsystem::Choose(const int32 Index)
{
	const FFVDialogueChoice* Choice = Choices.FindByPredicate([Index](const FFVDialogueChoice& It) { return It.Index == Index; });
	if (!Choice || !Choice->bAvailable || !ChoiceMade.IsBound())
	{
		return false;
	}

	FFVDialogueChoiceMade Callback = MoveTemp(ChoiceMade);
	ChoiceMade.Unbind();
	Choices.Reset();

	Callback.Execute(Index);
	return true;
}

void UFVDialogueSubsystem::CancelPending()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LineTimer);
	}
	StopVoice();
	LineFinished.Unbind();
	ChoiceMade.Unbind();
}

void UFVDialogueSubsystem::Bark(AActor* Speaker, const FText& Text, const float Duration)
{
	if (UFVDialogueParticipantComponent* Participant = UFVDialogueParticipantComponent::Find(Speaker))
	{
		Participant->OnBark.Broadcast(Text, Duration);
	}
	OnBark.Broadcast(Speaker, Text, Duration);
}

FName UFVDialogueSubsystem::GetLastBark(const AActor* Speaker) const
{
	return LastBarks.FindRef(const_cast<AActor*>(Speaker));
}

void UFVDialogueSubsystem::SetLastBark(AActor* Speaker, const FName Row)
{
	for (auto It = LastBarks.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
	LastBarks.Add(Speaker, Row);
}

void UFVDialogueSubsystem::StopVoice()
{
	if (UAudioComponent* Audio = Voice.Get())
	{
		Audio->Stop();
	}
	Voice.Reset();
}

float UFVDialogueSubsystem::ComputeDuration(const FFVDialogueLine& Line) const
{
	if (Line.Duration > 0.f)
	{
		return Line.Duration;
	}

	const UFVDialogueSettings* Settings = GetDefault<UFVDialogueSettings>();
	if (Line.Voice && Line.Voice->GetDuration() < INDEFINITELY_LOOPING_DURATION)
	{
		return Line.Voice->GetDuration() + Settings->VoicePadding;
	}
	return FMath::Max(Settings->MinLineDuration, Line.Text.ToString().Len() * Settings->SecondsPerCharacter);
}
