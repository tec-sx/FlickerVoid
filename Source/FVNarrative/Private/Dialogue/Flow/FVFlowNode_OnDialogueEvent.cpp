#include "Dialogue/Flow/FVFlowNode_OnDialogueEvent.h"

#include "FVNarrativeTags.h"
#include "Dialogue/FVDialogueMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNode_OnDialogueEvent)

FName UFVFlowNode_OnDialogueEvent::INPIN_In    = TEXT("In");
FName UFVFlowNode_OnDialogueEvent::OUTPIN_Out  = TEXT("Out");
FName UFVFlowNode_OnDialogueEvent::OUTPIN_Event = TEXT("Event");

UFVFlowNode_OnDialogueEvent::UFVFlowNode_OnDialogueEvent()
{
#if WITH_EDITOR
	Category = TEXT("Dialogue");
#endif
	
	InputPins = {FFlowPin(INPIN_In)};
	OutputPins = {FFlowPin(OUTPIN_Out), FFlowPin(OUTPIN_Event)};
}

void UFVFlowNode_OnDialogueEvent::ExecuteInput(const FName& PinName)
{
	if (PinName != INPIN_In)
	{
		return;
	}
	
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	Handle = Router.RegisterListener<FFVDialogueEventMessage>(
		FVNarrativeTags::Dialogue_Event, 
		this, 
		&UFVFlowNode_OnDialogueEvent::OnEventMessage);

	TriggerOutput(OUTPIN_Out, false);
}

void UFVFlowNode_OnDialogueEvent::Cleanup()
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(this);
	if (Handle.IsValid())
	{
		Router.UnregisterListener(Handle);
	}
	Handle = {};
	Super::Cleanup();
}

void UFVFlowNode_OnDialogueEvent::OnEventMessage(FGameplayTag Channel, const FFVDialogueEventMessage& Message)
{
	if (EventName.IsNone() || Message.EventName == EventName)
	{
		TriggerOutput(OUTPIN_Event, true);
	}
}
