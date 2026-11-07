#include "Nodes/FVDialogueFlowNodes.h"

#include "Components/AudioComponent.h"
#include "Data/FVCharacterDefinition.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "FlowAsset.h"
#include "FVDialogueSubsystem.h"
#include "FVDialogueSystem.h"
#include "FVFlowContext.h"
#include "GameplayTagAssetInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueFlowNodes)

namespace
{
	/** Starts loading a node's content in the background; the node hears back through NotifyPreloadComplete. */
	EFlowPreloadResult RequestPreload(UFlowNode* Node, const TArray<FSoftObjectPath>& Paths, TSharedPtr<FStreamableHandle>& OutHandle)
	{
		if (Paths.IsEmpty())
		{
			return EFlowPreloadResult::Completed;
		}

		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		const FStreamableDelegate OnLoaded = FStreamableDelegate::CreateUObject(Node, &UFlowNode::NotifyPreloadComplete);
		OutHandle = Streamable.RequestAsyncLoad(Paths, OnLoaded);
		if (OutHandle.IsValid() && !OutHandle->HasLoadCompleted())
		{
			return EFlowPreloadResult::PreloadInProgress;
		}
		return EFlowPreloadResult::Completed;
	}

	void ReleasePreload(TSharedPtr<FStreamableHandle>& Handle)
	{
		if (Handle.IsValid())
		{
			Handle->ReleaseHandle();
		}
		Handle.Reset();
	}

	/** The preloaded voice, or a synchronous load with a warning when the preload hasn't finished yet. */
	USoundBase* ResolveVoice(const TSoftObjectPtr<USoundBase>& Voice)
	{
		USoundBase* Sound = Voice.Get();
		if (Sound || Voice.IsNull())
		{
			return Sound;
		}

		UE_LOG(LogFVDialogueSystem, Warning, TEXT("Voice %s wasn't preloaded; loading it now."), *Voice.ToString());
		return Voice.LoadSynchronous();
	}
}

UFVFlowNode_DialogueBase::UFVFlowNode_DialogueBase()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

UFVDialogueSubsystem* UFVFlowNode_DialogueBase::EnsureConversation()
{
	UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this);
	if (Subsystem && !Subsystem->IsInConversation())
	{
		UFlowAsset* Flow = GetFlowAsset();
		AActor* FlowOwner = TryGetRootFlowActorOwner();
		UE_LOG(LogFVDialogueSystem, Warning, TEXT("%s started a conversation outside Start Conversation; it ends at End Conversation or when %s stops."),
			*GetName(), *GetNameSafe(Flow));
		Subsystem->BeginConversation(nullptr, FlowOwner, Flow);
	}
	return Subsystem;
}

void UFVFlowNode_DialogueBase::Cleanup()
{
	if (bPending)
	{
		bPending = false;
		if (UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this))
		{
			Subsystem->CancelPending();
		}
	}
	Super::Cleanup();
}

UFVFlowNode_DialogueLine::UFVFlowNode_DialogueLine()
{
}

void UFVFlowNode_DialogueLine::ExecuteInput(const FName& PinName)
{
	if (DispatchExecuteInputToPreloadHelper(PinName))
	{
		return;
	}

	UFVDialogueSubsystem* Subsystem = EnsureConversation();
	if (!Subsystem)
	{
		LogError(TEXT("No dialogue subsystem"));
		TriggerFirstOutput(true);
		return;
	}

	FFVDialogueLine Line;
	Line.Speaker = Speaker;
	Line.SpeakerActor = Subsystem->ResolveSpeaker(Speaker);
	Line.Text = Text;
	Line.Voice = ResolveVoice(Voice);
	Line.Mood = Mood;
	Line.Duration = Duration;
	Line.bSkippable = bSkippable;
	Line.bWaitForInput = bWaitForInput;

	bPending = true;
	Subsystem->PlayLine(Line, FFVDialogueLineFinished::CreateUObject(this, &UFVFlowNode_DialogueLine::HandleFinished));
}

EFlowPreloadResult UFVFlowNode_DialogueLine::PreloadContent()
{
	TArray<FSoftObjectPath> Paths;
	if (!Voice.IsNull())
	{
		Paths.Add(Voice.ToSoftObjectPath());
	}
	return RequestPreload(this, Paths, VoiceHandle);
}

void UFVFlowNode_DialogueLine::FlushContent()
{
	ReleasePreload(VoiceHandle);
}

void UFVFlowNode_DialogueLine::HandleFinished()
{
	bPending = false;
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_DialogueLine::GetNodeDescription() const
{
	const FString Name = Speaker ? Speaker->Display.Name.ToString() : TEXT("Owner");
	return FString::Printf(TEXT("%s: %s"), *Name, *Text.ToString());
}
#endif

UFVFlowNode_DialogueChoice::UFVFlowNode_DialogueChoice()
{
	OutputPins.Empty();
}

FName UFVFlowNode_DialogueChoice::GetOptionPinName(const int32 Index)
{
	return *FString::Printf(TEXT("Option%d"), Index);
}

void UFVFlowNode_DialogueChoice::ExecuteInput(const FName& PinName)
{
	UFVDialogueSubsystem* Subsystem = EnsureConversation();
	if (!Subsystem)
	{
		LogError(TEXT("No dialogue subsystem"));
		Finish();
		return;
	}

	const FFVConditionContext Context = FVFlow::MakeContext(*this);
	TArray<FFVDialogueChoice> Choices;
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		const FFVDialogueChoiceOption& Option = Options[Index];
		const bool bAvailable = Option.Conditions.Evaluate(Context);
		if (!bAvailable && Option.Conditions.FailurePresentation == EFVConditionFailurePresentation::Hidden)
		{
			continue;
		}

		FFVDialogueChoice& Choice = Choices.AddDefaulted_GetRef();
		Choice.Index = Index;
		Choice.Text = Option.Text;
		Choice.bAvailable = bAvailable;
		if (!bAvailable && Option.Conditions.FailurePresentation == EFVConditionFailurePresentation::ShowLockedWithReason)
		{
			Choice.LockedReason = Option.Conditions.FailureReason.IsEmpty() ? Option.Conditions.GetDescription() : Option.Conditions.FailureReason;
		}
	}

	if (!Choices.ContainsByPredicate([](const FFVDialogueChoice& Choice) { return Choice.bAvailable; }))
	{
		LogError(TEXT("No available choice; ending conversation"));
		Subsystem->EndConversation();
		return;
	}

	bPending = true;
	Subsystem->PresentChoices(Choices, FFVDialogueChoiceMade::CreateUObject(this, &UFVFlowNode_DialogueChoice::HandleChosen));
}

void UFVFlowNode_DialogueChoice::HandleChosen(const int32 Index)
{
	bPending = false;
	if (!Options.IsValidIndex(Index))
	{
		Finish();
		return;
	}

	Options[Index].OnChosen.Apply(FVFlow::MakeContext(*this));
	TriggerOutput(GetOptionPinName(Index), true);
}

#if WITH_EDITOR
TArray<FFlowPin> UFVFlowNode_DialogueChoice::GetContextOutputs() const
{
	TArray<FFlowPin> Pins = Super::GetContextOutputs();
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		const FText Label = Options[Index].Text.IsEmpty() ? FText::AsNumber(Index) : Options[Index].Text;
		Pins.Add(FFlowPin(GetOptionPinName(Index), Label));
	}
	return Pins;
}

void UFVFlowNode_DialogueChoice::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UFVFlowNode_DialogueChoice, Options))
	{
		RequestReconstruction();
	}
}

FString UFVFlowNode_DialogueChoice::GetNodeDescription() const
{
	return FString::Printf(TEXT("%d options"), Options.Num());
}
#endif

UFVFlowNode_EndConversation::UFVFlowNode_EndConversation()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	OutputPins.Empty();
}

void UFVFlowNode_EndConversation::ExecuteInput(const FName& PinName)
{
	if (UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this))
	{
		Subsystem->EndConversationFromFlow();
	}
	Finish();
}

UFVFlowNode_Bark::UFVFlowNode_Bark()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFVFlowNode_Bark::ExecuteInput(const FName& PinName)
{
	if (DispatchExecuteInputToPreloadHelper(PinName))
	{
		return;
	}

	AActor* Speaker = TryGetRootFlowActorOwner();
	FName RowName;
	const FFVBarkTableRow* Row = Barks && Speaker ? PickRow(Speaker, RowName) : nullptr;
	if (!Row)
	{
		TriggerFirstOutput(true);
		return;
	}

	UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this);
	if (Subsystem)
	{
		Subsystem->SetLastBark(Speaker, RowName);
	}

	float WaitTime = Row->DisplayDuration;
	if (USoundBase* Sound = ResolveVoice(Row->Voice))
	{
		PlayingVoice = UGameplayStatics::SpawnSoundAttached(Sound, Speaker->GetRootComponent());
		WaitTime = FMath::Max(WaitTime, Sound->GetDuration());
	}

	if (Subsystem)
	{
		Subsystem->Bark(Speaker, Row->Text, WaitTime);
	}

	if (!bWaitForDuration || WaitTime <= 0.f)
	{
		TriggerFirstOutput(true);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UFVFlowNode_Bark::HandleFinished, WaitTime, false);
}

const FFVBarkTableRow* UFVFlowNode_Bark::PickRow(AActor* Speaker, FName& OutRowName) const
{
	FGameplayTagContainer SpeakerTags;
	if (const IGameplayTagAssetInterface* TagOwner = Cast<IGameplayTagAssetInterface>(Speaker))
	{
		TagOwner->GetOwnedGameplayTags(SpeakerTags);
	}

	FGameplayTagContainer Combined = SpeakerTags;
	if (const IGameplayTagAssetInterface* PlayerTagOwner = Cast<IGameplayTagAssetInterface>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		FGameplayTagContainer PlayerTags;
		PlayerTagOwner->GetOwnedGameplayTags(PlayerTags);
		Combined.AppendTags(PlayerTags);
	}

	const FFVConditionContext Context = FVFlow::MakeContext(*this);
	TArray<TPair<FName, const FFVBarkTableRow*>> Valid;
	Barks->ForeachRow<FFVBarkTableRow>(TEXT("Bark"), [&](const FName& Name, const FFVBarkTableRow& Row)
	{
		const bool bIdentity = !Row.IdentityTag.IsValid() || SpeakerTags.HasTag(Row.IdentityTag);
		if (bIdentity && Combined.HasAll(Row.RequiredTags) && !Combined.HasAny(Row.BlockingTags) && Row.Conditions.Evaluate(Context))
		{
			Valid.Emplace(Name, &Row);
		}
	});

	const UFVDialogueSubsystem* Subsystem = UFVDialogueSubsystem::Get(this);
	if (const FName Last = Subsystem ? Subsystem->GetLastBark(Speaker) : NAME_None; !Last.IsNone() && Valid.Num() > 1)
	{
		Valid.RemoveAll([Last](const TPair<FName, const FFVBarkTableRow*>& Pair) { return Pair.Key == Last; });
	}

	if (Valid.IsEmpty())
	{
		return nullptr;
	}

	const TPair<FName, const FFVBarkTableRow*>& Picked = Valid[FMath::RandRange(0, Valid.Num() - 1)];
	OutRowName = Picked.Key;
	return Picked.Value;
}

void UFVFlowNode_Bark::HandleFinished()
{
	PlayingVoice.Reset();
	TriggerFirstOutput(true);
}

void UFVFlowNode_Bark::Cleanup()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer);
	}
	if (UAudioComponent* Audio = PlayingVoice.Get())
	{
		Audio->Stop();
	}
	PlayingVoice.Reset();
	Super::Cleanup();
}

EFlowPreloadResult UFVFlowNode_Bark::PreloadContent()
{
	TArray<FSoftObjectPath> Paths;
	if (Barks)
	{
		TArray<FFVBarkTableRow*> Rows;
		Barks->GetAllRows<FFVBarkTableRow>(TEXT("Bark preload"), Rows);
		for (const FFVBarkTableRow* Row : Rows)
		{
			if (Row && !Row->Voice.IsNull())
			{
				Paths.AddUnique(Row->Voice.ToSoftObjectPath());
			}
		}
	}
	return RequestPreload(this, Paths, VoicesHandle);
}

void UFVFlowNode_Bark::FlushContent()
{
	ReleasePreload(VoicesHandle);
}

#if WITH_EDITOR
FString UFVFlowNode_Bark::GetNodeDescription() const
{
	return Barks ? Barks->GetName() : TEXT("No bark table");
}

EDataValidationResult UFVFlowNode_Bark::ValidateNode()
{
	if (!Barks)
	{
		ValidationLog.Error<UFlowNode>(TEXT("Bark table is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif
