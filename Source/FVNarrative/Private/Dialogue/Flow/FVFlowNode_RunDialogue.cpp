#include "Dialogue/Flow/FVFlowNode_RunDialogue.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "FlowAsset.h"
#include "FVNarrativeTags.h"
#include "GameplayTagAssetInterface.h"
#include "SUDSDialogue.h"
#include "SUDSLibrary.h"
#include "SUDSScript.h"
#include "Dialogue/FVDialogueDirector.h"
#include "Dialogue/FVDialogueMessageTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_RunDialogue)

FName UFVFlowNode_RunDialogue::INPIN_In                 = TEXT("In");
FName UFVFlowNode_RunDialogue::OUTPIN_Out               = TEXT("Out");
FName UFVFlowNode_RunDialogue::OUTPIN_OnSpeakerLine     = TEXT("OnSpeakerLine");
FName UFVFlowNode_RunDialogue::OUTPIN_OnChoices         = TEXT("OnChoices");
FName UFVFlowNode_RunDialogue::OUTPIN_OnEvent           = TEXT("OnEvent");

UFVFlowNode_RunDialogue::UFVFlowNode_RunDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	
	InputPins  = { FFlowPin(INPIN_In) };
	OutputPins = {
		FFlowPin(OUTPIN_Out),
		FFlowPin(OUTPIN_OnSpeakerLine),
		FFlowPin(OUTPIN_OnChoices),
		FFlowPin(OUTPIN_OnEvent),
	};
}

void UFVFlowNode_RunDialogue::ExecuteInput(const FName& PinName)
{
	if (PinName == INPIN_In)
	{
		UFVDialogueDirector* Director = UFVDialogueDirector::Get(this);
		
		if (Director && Director->HasActiveDialogue())
		{
			FV_LOG_ERROR(LogFVNarrative, "Another dialogue is already running");
			return;
		}
		
		StartDialogue();
	}
}

void UFVFlowNode_RunDialogue::StartDialogue()
{
	if (!DialogueSet || DialogueSet->Script.IsNull())
	{
		FV_LOG_ERROR(LogFVNarrative,"No Dialogue Set or Script assigned");
		TriggerOutput(OUTPIN_Out, true);
		return;
	}
	
	USUDSScript* Script = DialogueSet->Script.LoadSynchronous();
	if (!Script)
	{
		LogError(TEXT("Failed to load SUDS script"));
		TriggerOutput(OUTPIN_Out, true);
		return;
	}
	
	AActor* Owner = ResolveOwnerActor();
	AActor* Player = ResolvePlayerActor();
	if (!Owner)
	{
		LogError(TEXT("Could not resolve dialogue owner actor"));
		TriggerOutput(OUTPIN_Out, true);
		return;
	}
	
	TArray<UObject*> Participants;
	Participants.Add(Owner);
	if (Player)
	{
		Participants.Add(Player);
	}
	
	Dialogue = USUDSLibrary::CreateDialogueWithParticipants(Owner, Script, Participants, false);
	if (!Dialogue)
	{
		LogError(TEXT("CreateDialogueWithParticipants failed"));
		TriggerOutput(OUTPIN_Out, true);
		return;
	}

	Dialogue->OnSpeakerLine.AddDynamic(this, &UFVFlowNode_RunDialogue::OnSpeakerLine);
	Dialogue->OnFinished.AddDynamic(this, &UFVFlowNode_RunDialogue::OnFinished);
	Dialogue->OnEvent.AddDynamic(this, &UFVFlowNode_RunDialogue::OnEvent);

	PushInitialVariables(Dialogue, Owner, Player);
	RegisterUIListeners();

	if (UFVDialogueDirector* Dir = UFVDialogueDirector::Get(this))
	{
		Dir->SetActiveDialogue(Dialogue, Owner, Player, DialogueSet);
	}

	Dialogue->Start();
}

void UFVFlowNode_RunDialogue::PushInitialVariables(USUDSDialogue* InDialogue, AActor* Owner, AActor* Player)
{
	if (!InDialogue || !DialogueSet)
	{
		return;
	}

	auto PushFromActor = [&](AActor* Actor)
	{
		if (!Actor) return;
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			for (const FGameplayAttribute& Attr : DialogueSet->AttributesToPush)
			{
				if (Attr.IsValid())
				{
					InDialogue->SetVariableFloat(FName(Attr.GetName()), ASC->GetNumericAttribute(Attr));
				}
			}
		}
	};

	PushFromActor(Owner);
	PushFromActor(Player);

	if (const IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(Owner))
	{
		FGameplayTagContainer Owned;
		TagIface->GetOwnedGameplayTags(Owned);
		InDialogue->SetVariableBoolean(
			TEXT("HaveMet"),
			Owned.HasTag(FGameplayTag::RequestGameplayTag(TEXT("NPC.HaveMetPlayer"))));
	}
}

AActor* UFVFlowNode_RunDialogue::ResolveOwnerActor() const
{
	if (AActor* RootOwner = TryGetRootFlowActorOwner())
	{
		return RootOwner;
	}
	
	// if (UFlowSubsystem* FS = GetFlowSubsystem())
	// {
	// 	TArray<UFlowComponent*> Comps = FS->GetComponents<UFlowComponent>(IdentityTags);
	// 	if (Comps.Num() > 0 && Comps[0])
	// 	{
	// 		return Comps[0]->GetOwner();
	// 	}
	// }
	return nullptr;
}

AActor* UFVFlowNode_RunDialogue::ResolvePlayerActor() const
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

void UFVFlowNode_RunDialogue::BroadcastLine() const
{
	if (!Dialogue)
	{
		return;
	}

	const UFVDialogueDirector* Director = UFVDialogueDirector::Get(this);

	FFVDialogueLineMessage Msg;
	Msg.SpeakerID = FName(Dialogue->GetSpeakerID());
	Msg.SpeakerDisplayName = Dialogue->GetSpeakerDisplayName();
	Msg.Text = Dialogue->GetText();
	Msg.bIsVoiced = Dialogue->IsCurrentLineVoiced();
	Msg.OwnerActor = Director ? Director->GetActiveNPC() : nullptr;
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_LineReady, Msg);
}

void UFVFlowNode_RunDialogue::BroadcastChoices() const
{
	FFVDialogueChoicesMessage Msg;
	BuildChoiceList(Msg.Choices);
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_ChoicesReady, Msg);
}

void UFVFlowNode_RunDialogue::BuildChoiceList(TArray<FFVDialogueChoiceMessage>& OutChoices) const
{
	OutChoices.Reset();
	if (!Dialogue)
	{
		return;
	}

	const int32 Num = Dialogue->GetNumberOfChoices();
	for (int32 i = 0; i < Num; ++i)
	{
		FFVDialogueChoiceMessage Choice;
		Choice.Index = i;
		Choice.Text = Dialogue->GetChoiceText(i);
		Choice.bIsEnabled = true;
		Choice.bWasTakenBefore = Dialogue->HasChoiceIndexBeenTakenPreviously(i);
		OutChoices.Add(Choice);
	}

	if (!DialogueSet)
	{
		return;
	}

	AActor* Player = UFVDialogueDirector::Get(this) ? UFVDialogueDirector::Get(this)->GetActivePlayer() : nullptr;
	UAbilitySystemComponent* ASC = Player ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player) : nullptr;

	for (const FDialogueChoiceMeta& Meta : DialogueSet->ChoiceMeta)
	{
		if (!Meta.bHasRequirement)
		{
			continue;
		}

		const bool bAlreadyPresent = OutChoices.ContainsByPredicate([&](const FFVDialogueChoiceMessage& E)
		{
			return E.Text.EqualTo(Meta.DisplayText);
		});
		if (bAlreadyPresent)
		{
			continue;
		}

		bool bMeets = true;
		if (ASC && Meta.RequiredAttribute.IsValid())
		{
			const float Val = ASC->GetNumericAttribute(Meta.RequiredAttribute);
			switch (Meta.CompareOp)
			{
			case EDialogueCompareOp::GreaterOrEqual: bMeets = Val >= Meta.RequiredValue; break;
			case EDialogueCompareOp::Greater:        bMeets = Val >  Meta.RequiredValue; break;
			case EDialogueCompareOp::LessOrEqual:    bMeets = Val <= Meta.RequiredValue; break;
			case EDialogueCompareOp::Less:           bMeets = Val <  Meta.RequiredValue; break;
			case EDialogueCompareOp::Equal:          bMeets = FMath::IsNearlyEqual(Val, Meta.RequiredValue); break;
			}
		}

		if (!bMeets)
		{
			FFVDialogueChoiceMessage Locked;
			Locked.Index = INDEX_NONE;
			Locked.ChoiceId = Meta.ChoiceId;
			Locked.Text = Meta.DisplayText;
			Locked.bIsEnabled = false;
			Locked.RequirementTooltip = Meta.RequirementTooltip;
			OutChoices.Add(Locked);
		}
	}
}

void UFVFlowNode_RunDialogue::RegisterUIListeners()
{
	UnregisterUIListeners();
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	
	ChoiceListenerHandle = Router.RegisterListener<FFVDialogueSubmitChoiceMessage>(
		FVNarrativeTags::Dialogue_SubmitChoice, this, &UFVFlowNode_RunDialogue::OnSubmitChoice);
	ContinueListenerHandle = Router.RegisterListener<FFVDialogueEndedMessage>(
		FVNarrativeTags::Dialogue_Continue, this, &UFVFlowNode_RunDialogue::OnSubmitContinue);
}

void UFVFlowNode_RunDialogue::UnregisterUIListeners()
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	
	if (ChoiceListenerHandle.IsValid())
	{
		Router.UnregisterListener(ChoiceListenerHandle);
	}
	if (ContinueListenerHandle.IsValid())
	{
		Router.UnregisterListener(ContinueListenerHandle);
	}
	
	ChoiceListenerHandle = {};
	ContinueListenerHandle = {};
}

void UFVFlowNode_RunDialogue::OnSpeakerLine(USUDSDialogue* InDialogue)
{
	BroadcastLine();
	TriggerOutput(OUTPIN_OnSpeakerLine, false);

	if (InDialogue && InDialogue->GetNumberOfChoices() > 0 && !InDialogue->IsSimpleContinue())
	{
		BroadcastChoices();
		TriggerOutput(OUTPIN_OnChoices, false);
	}
}

void UFVFlowNode_RunDialogue::OnFinished(USUDSDialogue* InDialogue)
{
	UnregisterUIListeners();

	if (UFVDialogueDirector* Director = UFVDialogueDirector::Get(this))
	{
		Director->PersistActiveVariables();
		Director->ClearActiveDialogue();
	}

	FFVDialogueEndedMessage Msg;
	Msg.bWasAborted = false;
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Ended, Msg);

	Dialogue = nullptr;
	TriggerOutput(OUTPIN_Out, true);
}

void UFVFlowNode_RunDialogue::OnEvent(USUDSDialogue* InDialogue, FName EventName, const TArray<FSUDSValue>& Arguments)
{
	FFVDialogueEventMessage Msg;
	Msg.EventName = EventName;
	for (const FSUDSValue& Value : Arguments)
	{
		switch (Value.GetType())
		{
		case ESUDSValueType::Text:    Msg.Arguments.Add(Value.GetTextValue().ToString()); break;
		case ESUDSValueType::Name:    Msg.Arguments.Add(Value.GetNameValue().ToString()); break;
		case ESUDSValueType::Int:     Msg.Arguments.Add(FString::FromInt(Value.GetIntValue())); break;
		case ESUDSValueType::Float:   Msg.Arguments.Add(FString::SanitizeFloat(Value.GetFloatValue())); break;
		case ESUDSValueType::Boolean: Msg.Arguments.Add(Value.GetBooleanValue() ? TEXT("true") : TEXT("false")); break;
		default: break;
		}
	}
	
	UFVDialogueDirector* Director = UFVDialogueDirector::Get(this);
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Event, Msg);

	TriggerOutput(OUTPIN_OnEvent, false);

	// Built-in mini-game shortcut: end dialogue and start action
	if (EventName == TEXT("StartContextualAction") && Arguments.Num() > 0)
	{
		const FName ContextActionId = Arguments[0].GetNameValue();
		UnregisterUIListeners();

		FFVDialogueEndedMessage EndMsg;
		EndMsg.bWasAborted = false;
		EndMsg.bStartedContextAction = true;
		EndMsg.ContextActionId = ContextActionId;
		
		Router.BroadcastMessage(FVNarrativeTags::Dialogue_Ended, EndMsg);

		if (Director)
		{
			Director->PersistActiveVariables();
			Director->RestoreGameplayCamera();
			Director->StartContextualAction(ContextActionId);
			Director->ClearActiveDialogue();
		}

		if (Dialogue && !Dialogue->IsEnded())
		{
			Dialogue->End(true);
		}
		Dialogue = nullptr;
		TriggerOutput(OUTPIN_Out, true);
		return;
	}
	
	if (EventName == TEXT("CameraShot") && Arguments.Num() > 0)
	{
		if (Director)
		{
			Director->BlendToShot(Arguments[0].GetNameValue(), Director->GetActiveNPC());
		}
	}
}

void UFVFlowNode_RunDialogue::OnSubmitChoice(FGameplayTag Channel, const FFVDialogueSubmitChoiceMessage& Message)
{
	if (Dialogue && !Dialogue->IsEnded() && Message.ChoiceIndex != INDEX_NONE)
	{
		Dialogue->Choose(Message.ChoiceIndex);
	}
}

void UFVFlowNode_RunDialogue::OnSubmitContinue(FGameplayTag Channel, const FFVDialogueEndedMessage& Message)
{
	if (Dialogue && !Dialogue->IsEnded())
	{
		Dialogue->Continue();
	}
}

void UFVFlowNode_RunDialogue::Cleanup()
{
	UnregisterUIListeners();

	if (Dialogue)
	{
		Dialogue->OnSpeakerLine.RemoveAll(this);
		Dialogue->OnFinished.RemoveAll(this);
		Dialogue->OnEvent.RemoveAll(this);

		if (!Dialogue->IsEnded())
		{
			Dialogue->End(true);
		}
		Dialogue = nullptr;
	}

	if (UFVDialogueDirector* Director = UFVDialogueDirector::Get(this))
	{
		if (Director->GetActiveDialogue() == nullptr || Director->GetActiveDialogue() == Dialogue)
		{
			Director->ClearActiveDialogue();
		}
	}

	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_RunDialogue::GetNodeDescription() const
{
	return DialogueSet ? DialogueSet->GetName() : TEXT("No DialogueSet");
}

EDataValidationResult UFVFlowNode_RunDialogue::ValidateNode()
{
	if (!DialogueSet)
	{
		ValidationLog.Error(TEXT("DialogueSet is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}

FString UFVFlowNode_RunDialogue::GetStatusString() const
{
	if (!Dialogue)
	{
		return FString();
	}
	UFVDialogueDirector* Dir = UFVDialogueDirector::Get(this);
	return FString::Printf(TEXT("%s"), *Dialogue->GetSpeakerID());
}
#endif
