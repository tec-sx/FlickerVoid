// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Dialogue/Flow/FVFlowNode_CallOut.h"

#include "FlowComponent.h"
#include "FVNarrativeTags.h"
#include "GameplayTagAssetInterface.h"
#include "Components/AudioComponent.h"
#include "Dialogue/FVCallOutTableRow.h"
#include "Dialogue/FVDialogueMessageTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_CallOut)

TMap<TWeakObjectPtr<AActor>, FName> UFVFlowNode_CallOut::LastPlayedRowByNPC;
FName UFVFlowNode_CallOut::INPIN_In   = TEXT("In");
FName UFVFlowNode_CallOut::OUTPIN_Out = TEXT("Out");

UFVFlowNode_CallOut::UFVFlowNode_CallOut()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	InputPins  = { FFlowPin(INPIN_In) };
	OutputPins = { FFlowPin(OUTPIN_Out) };
}

void UFVFlowNode_CallOut::ExecuteInput(const FName& PinName)
{
	if (PinName == INPIN_In)
	{
		Play();
	}
}

void UFVFlowNode_CallOut::Cleanup()
{
	if (UWorld* World = GetWorld())
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

void UFVFlowNode_CallOut::Play()
{
	if (!CallOutDatabase)
	{
		LogError(TEXT("CallOut Table is not set"));
		TriggerOutput(OUTPIN_Out, true);
		return;
	}

	AActor* Owner  = ResolveOwnerActor();
	AActor* Player = ResolvePlayerActor();
	
	if (!Owner)
	{
		LogError(TEXT("Could not resolve CallOut owner"));
		TriggerOutput(OUTPIN_Out, true);
		return;
	}

	FName RowName;
	const FFVCallOutTableRow* Row = nullptr;
	if (!PickRow(Owner, Player, RowName, Row) || !Row)
	{
		TriggerOutput(OUTPIN_Out, true);
		return;
	}

	RememberPlayed(Owner, RowName);

	FFVDialogueCallOutMessage Msg;
	Msg.Text            = Row->Text;
	Msg.SpeakerID       = Owner->GetFName();
	Msg.OwnerActor      = Owner;
	Msg.Voice           = Row->Voice;
	Msg.DisplayDuration = Row->DisplayDuration > 0.f ? Row->DisplayDuration : 3.f;
	Msg.bShowTalkIcon   = Row->bShowTalkIcon;
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_CallOut, Msg);

	float WaitTime = Msg.DisplayDuration;

	if (!Row->Voice.IsNull())
	{
		if (USoundBase* Sound = Row->Voice.LoadSynchronous())
		{

			PlayingVoice = UGameplayStatics::SpawnSoundAtLocation(
				GetWorld(),
				Sound, 
				Owner->GetActorLocation(),
				Owner->GetActorRotation(),
				1,
				1,
				0,
				Sound->AttenuationSettings);

			if (PlayingVoice.IsValid())
			{
				WaitTime = FMath::Max(WaitTime, Sound->GetDuration());
			}
		}
	}

	if (bWaitForDuration && WaitTime > 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				CallOutTimerHandle, 
				this, 
				&UFVFlowNode_CallOut::OnCallOutFinished, 
				WaitTime, 
				false);
			return;
		}
	}

	TriggerOutput(OUTPIN_Out, true);
}

bool UFVFlowNode_CallOut::IsRowValid(
	const FName& RowName, 
	const FFVCallOutTableRow& Row,
	const FGameplayTagContainer& CombinedTags, 
	const FGameplayTagContainer& NPCTags) const
{
	if (Row.IdentityTag.IsValid() && !NPCTags.HasTag(Row.IdentityTag))
	{
		return false;
	}
	
	FGameplayTagContainer Required = Row.RequiredTags;
	Required.AppendTags(ContextTags);

	if (Required.Num() > 0 && !CombinedTags.HasAll(Required))
	{
		return false;
	}

	if (Row.BlockingTags.Num() > 0 && CombinedTags.HasAny(Row.BlockingTags))
	{
		return false;
	}

	return true;
}

bool UFVFlowNode_CallOut::PickRow(
	AActor* NPC, 
	AActor* Player, 
	FName& OutRowName,
	const FFVCallOutTableRow*& OutRow) const
{
	OutRowName = NAME_None;
	OutRow = nullptr;

	if (!CallOutDatabase)
	{
		return false;
	}

	FGameplayTagContainer NPCTags;
	FGameplayTagContainer PlayerTags;
	GatherTags(NPC, NPCTags);
	GatherTags(Player, PlayerTags);

	FGameplayTagContainer Combined = NPCTags;
	Combined.AppendTags(PlayerTags);

	TArray<FName> RowNames;
	CallOutDatabase->GetRowMap().GetKeys(RowNames);

	TArray<TPair<FName, const FFVCallOutTableRow*>> Valid;
	for (const FName& Name : RowNames)
	{
		const FFVCallOutTableRow* Row = CallOutDatabase->FindRow<FFVCallOutTableRow>(Name, TEXT("PlayCallOut"), false);
		if (Row && IsRowValid(Name, *Row, Combined, NPCTags))
		{
			Valid.Emplace(Name, Row);
		}
	}

	if (Valid.Num() == 0)
	{
		return false;
	}

	// Avoid immediate repeat when possible
	const FName Last = GetLastPlayed(NPC);
	if (Valid.Num() > 1 && Last != NAME_None)
	{
		Valid.RemoveAll([&](const TPair<FName, const FFVCallOutTableRow*>& Pair)
		{
			return Pair.Key == Last;
		});
	}

	const int32 Pick = FMath::RandRange(0, Valid.Num() - 1);
	OutRowName = Valid[Pick].Key;
	OutRow = Valid[Pick].Value;
	return OutRow != nullptr;
}

void UFVFlowNode_CallOut::RememberPlayed(AActor* NPC, FName RowName)
{
	if (NPC && RowName != NAME_None)
	{
		LastPlayedRowByNPC.FindOrAdd(NPC) = RowName;
	}
}

FName UFVFlowNode_CallOut::GetLastPlayed(AActor* NPC) const
{
	if (!NPC)
	{
		return NAME_None;
	}
	if (const FName* Found = LastPlayedRowByNPC.Find(NPC))
	{
		return *Found;
	}
	return NAME_None;
}

AActor* UFVFlowNode_CallOut::ResolveOwnerActor() const
{
	if (AActor* Root = TryGetRootFlowActorOwner())
	{
		return Root;
	}
	
	return nullptr;
}

AActor* UFVFlowNode_CallOut::ResolvePlayerActor() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PC = World->GetFirstPlayerController())
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}

void UFVFlowNode_CallOut::GatherTags(AActor* Actor, FGameplayTagContainer& OutTags) const
{
	OutTags.Reset();
	if (!Actor)
	{
		return;
	}
	if (const IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Actor))
	{
		TagIface->GetOwnedGameplayTags(OutTags);
	}
}

void UFVFlowNode_CallOut::OnCallOutFinished()
{
	CallOutTimerHandle.Invalidate();
	PlayingVoice.Reset();
	TriggerOutput(OUTPIN_Out, true);
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
		ValidationLog.Error(TEXT("CallOutDatabase is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif