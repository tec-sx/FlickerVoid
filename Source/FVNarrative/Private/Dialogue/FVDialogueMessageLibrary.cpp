
#include "Dialogue/FVDialogueMessageLibrary.h"

#include "FVNarrativeTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueMessageLibrary)

void UFVDialogueMessageLibrary::BroadcastLineReady(const UObject* WorldContextObject, const FFVDialogueLineMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_LineReady, Message);
}

void UFVDialogueMessageLibrary::BroadcastChoicesReady(const UObject* WorldContextObject,
	const FFVDialogueChoicesMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_ChoicesReady, Message);
}

void UFVDialogueMessageLibrary::BroadcastEvent(const UObject* WorldContextObject, const FFVDialogueEventMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Event, Message);
}

void UFVDialogueMessageLibrary::BroadcastDialogueEnded(const UObject* WorldContextObject, const FFVDialogueEndedMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Ended, Message);
}

void UFVDialogueMessageLibrary::BroadcastDialogueSubmitChoice(const UObject* WorldContextObject,
	const FFVDialogueSubmitChoiceMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_SubmitChoice, Message);
}

void UFVDialogueMessageLibrary::BroadcastDialogueContinue(const UObject* WorldContextObject,
	const FFVDialogueEndedMessage& Message)
{
	UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(WorldContextObject);
	Router.BroadcastMessage(FVNarrativeTags::Dialogue_Continue, Message);
}
