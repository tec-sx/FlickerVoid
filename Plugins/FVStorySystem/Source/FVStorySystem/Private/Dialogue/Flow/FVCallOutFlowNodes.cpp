#include "Dialogue/Flow/FVCallOutFlowNodes.h"

#include "Components/AudioComponent.h"
#include "Dialogue/FVDialogueBridge.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagAssetInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCallOutFlowNodes)

TMap<TWeakObjectPtr<AActor>, FName> UFVFlowNode_CallOut::LastPlayedRowByOwner;

UFVFlowNode_CallOut::UFVFlowNode_CallOut()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFVFlowNode_CallOut::ExecuteInput(const FName& PinName)
{
	AActor* Owner = TryGetRootFlowActorOwner();
	if (!CallOutDatabase || !Owner)
	{
		LogError(TEXT("CallOut needs a database and an owning actor"));
		TriggerFirstOutput(true);
		return;
	}

	FName RowName;
	const FFVCallOutTableRow* Row = nullptr;
	if (!PickRow(Owner, ResolvePlayerActor(), RowName, Row))
	{
		TriggerFirstOutput(true);
		return;
	}

	LastPlayedRowByOwner.FindOrAdd(Owner) = RowName;
	Broadcast(*Row, Owner);

	const float WaitTime = FMath::Max(PlayVoice(*Row, Owner), Row->DisplayDuration);
	if (!bWaitForDuration || WaitTime <= 0.f)
	{
		TriggerFirstOutput(true);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(CallOutTimerHandle, this, &UFVFlowNode_CallOut::OnCallOutFinished, WaitTime, false);
}

void UFVFlowNode_CallOut::Cleanup()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CallOutTimerHandle);
	}
	if (PlayingVoice.IsValid())
	{
		PlayingVoice->Stop();
	}
	PlayingVoice.Reset();
	Super::Cleanup();
}

bool UFVFlowNode_CallOut::PickRow(AActor* Owner, AActor* Player, FName& OutRowName, const FFVCallOutTableRow*& OutRow) const
{
	FGameplayTagContainer OwnerTags;
	FGameplayTagContainer PlayerTags;
	GatherTags(Owner, OwnerTags);
	GatherTags(Player, PlayerTags);

	FGameplayTagContainer Combined = OwnerTags;
	Combined.AppendTags(PlayerTags);

	TArray<TPair<FName, const FFVCallOutTableRow*>> Valid;
	CallOutDatabase->ForeachRow<FFVCallOutTableRow>(TEXT("PlayCallOut"), [&](const FName& Name, const FFVCallOutTableRow& Row)
	{
		if (IsRowValid(Row, Combined, OwnerTags))
		{
			Valid.Emplace(Name, &Row);
		}
	});

	const FName* Last = LastPlayedRowByOwner.Find(Owner);
	if (Valid.Num() > 1 && Last)
	{
		Valid.RemoveAll([Last](const TPair<FName, const FFVCallOutTableRow*>& Pair) { return Pair.Key == *Last; });
	}

	if (Valid.IsEmpty())
	{
		return false;
	}

	const int32 Pick = FMath::RandRange(0, Valid.Num() - 1);
	OutRowName = Valid[Pick].Key;
	OutRow = Valid[Pick].Value;
	return true;
}

bool UFVFlowNode_CallOut::IsRowValid(const FFVCallOutTableRow& Row, const FGameplayTagContainer& CombinedTags, const FGameplayTagContainer& OwnerTags) const
{
	if (Row.IdentityTag.IsValid() && !OwnerTags.HasTag(Row.IdentityTag))
	{
		return false;
	}
	if (!CombinedTags.HasAll(Row.RequiredTags) || CombinedTags.HasAny(Row.BlockingTags))
	{
		return false;
	}
	return Row.Conditions.Evaluate(FVDialogueBridge::MakeContext(GetWorld()));
}

float UFVFlowNode_CallOut::PlayVoice(const FFVCallOutTableRow& Row, AActor* Owner)
{
	USoundBase* Sound = Row.Voice.LoadSynchronous();
	if (!Sound)
	{
		return 0.f;
	}

	USoundAttenuation* Attenuation = NewObject<USoundAttenuation>(this);
	Attenuation->Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
	Attenuation->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	Attenuation->Attenuation.FalloffDistance = 3000.f;

	PlayingVoice = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Owner->GetActorLocation(), Owner->GetActorRotation(), 1.f, 1.f, 0.f, Attenuation);
	return PlayingVoice.IsValid() ? Sound->GetDuration() : 0.f;
}

void UFVFlowNode_CallOut::Broadcast(const FFVCallOutTableRow& Row, AActor* Owner) const
{
	FFVDialogueCallOutMessage Msg;
	Msg.Text = Row.Text;
	Msg.SpeakerID = Owner->GetFName();
	Msg.OwnerActor = Owner;
	Msg.Voice = Row.Voice;
	Msg.DisplayDuration = Row.DisplayDuration > 0.f ? Row.DisplayDuration : 3.f;
	Msg.bShowTalkIcon = Row.bShowTalkIcon;

	static const FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Dialogue.CallOut"));
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(Channel, Msg);
}

AActor* UFVFlowNode_CallOut::ResolvePlayerActor() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void UFVFlowNode_CallOut::GatherTags(const AActor* Actor, FGameplayTagContainer& OutTags)
{
	OutTags.Reset();
	if (const IGameplayTagAssetInterface* Tags = Cast<IGameplayTagAssetInterface>(Actor))
	{
		Tags->GetOwnedGameplayTags(OutTags);
	}
}

void UFVFlowNode_CallOut::OnCallOutFinished()
{
	CallOutTimerHandle.Invalidate();
	PlayingVoice.Reset();
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_CallOut::GetNodeDescription() const
{
	return CallOutDatabase ? CallOutDatabase->GetName() : TEXT("No CallOutDatabase");
}

EDataValidationResult UFVFlowNode_CallOut::ValidateNode()
{
	if (!CallOutDatabase)
	{
		ValidationLog.Error<UFlowNode>(TEXT("CallOutDatabase is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif
