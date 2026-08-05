// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogue/Flow/FVFlowNode_EndDialogue.h"

#include "FVNarrativeTags.h"
#include "SUDSDialogue.h"
#include "Dialogue/FVDialogueDirector.h"
#include "Dialogue/FVDialogueMessageTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_EndDialogue)

UFVFlowNode_EndDialogue::UFVFlowNode_EndDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	
	InputPins = { FFlowPin(TEXT("In")) };
	OutputPins = { FFlowPin(TEXT("Out")) };
}

void UFVFlowNode_EndDialogue::ExecuteInput(const FName& PinName)
{
	if (UFVDialogueDirector* Director = UFVDialogueDirector::Get(this))
	{
		if (USUDSDialogue* Dialogue = Director->GetActiveDialogue())
		{
			if (bPersistVariables)
			{
				Director->PersistActiveVariables();
			}
			if (!Dialogue->IsEnded())
			{
				Dialogue->End(true);
			}
		}

		if (bRestoreCamera)
		{
			Director->RestoreGameplayCamera();
		}

		Director->ClearActiveDialogue();
	}

	FFVDialogueEndedMessage Msg;
	Msg.bWasAborted = bWasAborted;
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Ended, Msg);

	TriggerFirstOutput(true);
}
