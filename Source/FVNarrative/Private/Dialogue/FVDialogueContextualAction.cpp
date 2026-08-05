#include "Dialogue/FVDialogueContextualAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueContextualAction)

void UFVDialogueContextualAction::StartAction_Implementation(AActor* Player, AActor* NPC, FName ActionId)
{
	PlayerActor = Player;
	NPCActor = NPC;
	CurrentActionId = ActionId;
	// Subclasses / Angelscript override and run the mini-games, animation, etc.
}

void UFVDialogueContextualAction::FinishAction(bool bSuccess)
{
	OnFinished.Broadcast(bSuccess);
}
