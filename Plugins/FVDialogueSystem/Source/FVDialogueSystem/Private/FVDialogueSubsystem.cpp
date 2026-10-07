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
	Super::Deinitialize();
}

bool UFVDialogueSubsystem::StartConversation(UFlowAsset* InDialogue, AActor* InInstigator, AActor* InOwner)
{
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UFlowSubsystem* Flow = GameInstance ? GameInstance->GetSubsystem<UFlowSubsystem>() : nullptr;
	if (bActive || !InDialogue || !InOwner || !Flow)
	{
		UE_LOG(LogFVDialogueSystem, Warning, TEXT("Can't start conversation %s with %s."), *GetNameSafe(InDialogue), *GetNameSafe(InOwner));
		return false;
	}

	BeginConversation(InInstigator, InOwner);
	Dialogue = InDialogue;
	Flow->StartRootFlow(InOwner, InDialogue, TScriptInterface<IFlowDataPinValueSupplierInterface>(), false);
	return true;
}

void UFVDialogueSubsystem::BeginConversation(AActor* InInstigator, AActor* InOwner)
{
	if (bActive)
	{
		return;
	}

	bActive = true;
	Instigator = InInstigator ? InInstigator : UGameplayStatics::GetPlayerPawn(this, 0);
	Owner = InOwner;
	OnConversationStarted.Broadcast();
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

	AActor* FlowOwner = Owner.Get();
	UFlowAsset* FlowTemplate = Dialogue;

	bActive = false;
	Instigator.Reset();
	Owner.Reset();
	Dialogue = nullptr;
	CurrentLine = FFVDialogueLine();
	Choices.Reset();

	// Deferred so it's safe to end from inside a running node.
	if (bAbortFlow && FlowOwner && FlowTemplate)
	{
		TWeakObjectPtr<AActor> WeakOwner = FlowOwner;
		TWeakObjectPtr<UFlowAsset> WeakTemplate = FlowTemplate;
		TWeakObjectPtr<UGameInstance> WeakGameInstance = GetWorld()->GetGameInstance();
		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakOwner, WeakTemplate, WeakGameInstance]()
		{
			UFlowSubsystem* Flow = WeakGameInstance.IsValid() ? WeakGameInstance->GetSubsystem<UFlowSubsystem>() : nullptr;
			if (Flow && WeakOwner.IsValid() && WeakTemplate.IsValid())
			{
				Flow->FinishRootFlow(WeakOwner.Get(), WeakTemplate.Get(), EFlowFinishPolicy::Abort);
			}
		});
	}

	OnConversationEnded.Broadcast();
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
