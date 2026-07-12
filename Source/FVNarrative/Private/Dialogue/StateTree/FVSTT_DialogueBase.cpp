#include "Dialogue/StateTree/FVSTT_DialogueBase.h"

#include "StateTreeExecutionContext.h"
#include "Dialogue/FVDialogueSubsystem.h"
#include "Dialogue/FVDialogueDatabaseSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

class UFVDialogueSubsystem;

static UFVDialogueSubsystem* GetDialogueSubsystem(const FStateTreeExecutionContext& Ctx)
{
	const UObject* Owner = Ctx.GetOwner();
	if (!Owner) return nullptr;
	const UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return UGameplayStatics::GetGameInstance(World)->GetSubsystem<UFVDialogueSubsystem>();
}

static UFVDialogueDatabaseSubsystem* GetDatabaseSubsystem(const FStateTreeExecutionContext& Ctx)
{
	const UObject* Owner = Ctx.GetOwner();
	if (!Owner) return nullptr;
	const UWorld* World = Owner->GetWorld();
	if (!World) return nullptr;
	return UGameplayStatics::GetGameInstance(World)->GetSubsystem<UFVDialogueDatabaseSubsystem>();
}