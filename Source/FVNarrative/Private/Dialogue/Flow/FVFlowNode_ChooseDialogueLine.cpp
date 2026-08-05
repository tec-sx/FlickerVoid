#include "Dialogue/Flow/FVFlowNode_ChooseDialogueLine.h"

#include "SUDSDialogue.h"
#include "Dialogue/FVDialogueDirector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_ChooseDialogueLine)

UFVFlowNode_ChooseDialogueLine::UFVFlowNode_ChooseDialogueLine()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	
	InputPins = { FFlowPin(TEXT("In")) };
	OutputPins = { FFlowPin(TEXT("Out")) };
}

void UFVFlowNode_ChooseDialogueLine::ExecuteInput(const FName& PinName)
{
	if (const UFVDialogueDirector* Director = UFVDialogueDirector::Get(this))
	{
		if (USUDSDialogue* Dialogue = Director->GetActiveDialogue())
		{
			if (!Dialogue->IsEnded())
			{
				Dialogue->Choose(ChoiceIndex);
			}
		}
	}
	TriggerFirstOutput(true);
}
