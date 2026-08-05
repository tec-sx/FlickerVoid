// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogue/Flow/FVFlowNode_ContinueDialogue.h"

#include "SUDSDialogue.h"
#include "Dialogue/FVDialogueDirector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_ContinueDialogue)

UFVFlowNode_ContinueDialogue::UFVFlowNode_ContinueDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	
	InputPins = { FFlowPin(TEXT("In")) };
	OutputPins = { FFlowPin(TEXT("Out")) };
}

void UFVFlowNode_ContinueDialogue::ExecuteInput(const FName& PinName)
{
	if (const UFVDialogueDirector* Director = UFVDialogueDirector::Get(this))
	{
		if (USUDSDialogue* Dialogue = Director->GetActiveDialogue())
		{
			if (!Dialogue->IsEnded())
			{
				Dialogue->Continue();
			}
		}
	}
	TriggerFirstOutput(true);
}
