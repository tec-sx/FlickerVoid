#include "Dialogue/FVDialogueDirector.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "Dialogue/FVDialogueContextualAction.h"
#include "Dialogue/FVDialogueMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueDirector)

UFVDialogueDirector* UFVDialogueDirector::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	return World ? World->GetSubsystem<UFVDialogueDirector>() : nullptr;
}

void UFVDialogueDirector::Deinitialize()
{
	ClearActiveDialogue();
	RestoreGameplayCamera();
	Super::Deinitialize();
}

void UFVDialogueDirector::SetActiveDialogue(USUDSDialogue* Dialogue, AActor* NPC, AActor* Player, UFVDialogueSet* DialogueSet)
{
	ActiveDialogue = Dialogue;
	ActiveNPC = NPC;
	ActivePlayer = Player;
	ActiveSet = DialogueSet;
}

void UFVDialogueDirector::ClearActiveDialogue()
{
	ActiveDialogue = nullptr;
	ActiveNPC.Reset();
	ActivePlayer.Reset();
	ActiveSet = nullptr;
}

void UFVDialogueDirector::PersistActiveVariables()
{
	if (!ActiveDialogue || !ActiveSet || !ActiveNPC.IsValid())
	{
		return;
	}

	// // Project-specific: write VariablesToPersist back to a component / tags / save.
	// // Example skeleton using gameplay tags:
	// if (IGameplayTagAssetInterface* TagIface = Cast<IGameplayTagAssetInterface>(ActiveNPC.Get()))
	// {
	// 	if (ActiveDialogue->IsVariableSet(TEXT("HaveMet")) && ActiveDialogue->GetVariableBoolean(TEXT("HaveMet")))
	// 	{
	// 		// Add tag via your tag manager / ASC
	// 	}
	// }
}

void UFVDialogueDirector::PlayEnterSequence()
{
	if (!ActiveSet || ActiveSet->EnterConversationSequence.IsNull())
	{
		return;
	}
	
	ULevelSequence* Sequence = ActiveSet->EnterConversationSequence.LoadSynchronous();
	if (!Sequence)
	{
		return;
	}
	
	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bAutoPlay = true;
	Settings.bPauseAtEnd = true;

	ALevelSequenceActor* OutActor = nullptr;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings, OutActor);
	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.AddDynamic(this, &UFVDialogueDirector::OnEnterSequenceFinished);
	}
}

void UFVDialogueDirector::OnEnterSequenceFinished()
{
	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.RemoveDynamic(this, &UFVDialogueDirector::OnEnterSequenceFinished);
		SequencePlayer = nullptr;
	}
}

void UFVDialogueDirector::BlendToShot(FName ShotName, AActor* FocusActor)
{
	if (!ActiveSet)
	{
		return;
	}
	
	const FDialogueCameraShot* Found = ActiveSet->CameraLibrary.FindByPredicate([&](const FDialogueCameraShot& S)
	{
		return S.ShotName == ShotName;
	});
	
	if (!Found)
	{
		return;
	}
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	if (!RuntimeCameraActor)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RuntimeCameraActor = GetWorld()->SpawnActor<ACineCameraActor>(ACineCameraActor::StaticClass(), Found->RelativeTransform, Params);
	}

	if (ACineCameraActor* Cam = Cast<ACineCameraActor>(RuntimeCameraActor))
	{
		Cam->SetActorTransform(Found->RelativeTransform);
		if (UCineCameraComponent* Comp = Cam->GetCineCameraComponent())
		{
			Comp->SetFieldOfView(Found->FOV);
		}
		PC->SetViewTargetWithBlend(Cam, Found->BlendTime);
	}
}

void UFVDialogueDirector::RestoreGameplayCamera()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->SetViewTargetWithBlend(PC->GetPawn(), 0.8f);
	}
	if (RuntimeCameraActor)
	{
		RuntimeCameraActor->Destroy();
		RuntimeCameraActor = nullptr;
	}
}

void UFVDialogueDirector::StartContextualAction(FName ContextActionId)
{
	if (!ActiveSet)
	{
		return;
	}
	
	const TSoftClassPtr<UFVDialogueContextualAction>* SoftClass = ActiveSet->ContextualActions.Find(ContextActionId);
	if (!SoftClass || SoftClass->IsNull())
	{
		return;
	}

	UClass* Cls = SoftClass->LoadSynchronous();
	if (!Cls)
	{
		return;
	}

	ActiveContextualAction = NewObject<UFVDialogueContextualAction>(this, Cls);
	ActiveContextualAction->StartAction(ActivePlayer.Get(), ActiveNPC.Get(), ContextActionId);
}
