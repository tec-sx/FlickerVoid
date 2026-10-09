#include "Cinematic/FVCinematic.h"

#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCinematic)

#define LOCTEXT_NAMESPACE "FVCinematic"

UFVCinematicSubsystem* UFVCinematicSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVCinematicSubsystem>() : nullptr;
}

bool UFVCinematicSubsystem::Play(UFVCinematicDefinition* Cinematic)
{
	if (!Cinematic)
	{
		return false;
	}

	ULevelSequence* Sequence = Cinematic->Sequence.LoadSynchronous();
	if (!Sequence)
	{
		return false;
	}

	if (IsPlaying())
	{
		Cleanup();
	}

	FMovieSceneSequencePlaybackSettings Settings;
	Settings.bHidePlayer = Cinematic->bHidePlayer;
	Settings.bDisableMovementInput = Cinematic->bDisablePlayerInput;
	Settings.bDisableLookAtInput = Cinematic->bDisablePlayerInput;

	ALevelSequenceActor* OutActor = nullptr;
	Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, Settings, OutActor);
	if (!Player)
	{
		return false;
	}

	SequenceActor = OutActor;
	Current = Cinematic;
	Player->OnFinished.AddDynamic(this, &UFVCinematicSubsystem::HandleFinished);
	SetPlayerState(true);
	Player->Play();
	OnStarted.Broadcast();
	return true;
}

void UFVCinematicSubsystem::Skip()
{
	if (CanSkip() && Player)
	{
		Player->GoToEndAndStop();
	}
}

void UFVCinematicSubsystem::HandleFinished()
{
	UFVCinematicDefinition* Finished = Current;
	Cleanup();
	if (!Finished)
	{
		return;
	}

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this); Facts && Finished->Id.IsValid())
	{
		Facts->SetFact(Finished->Id, 1);
	}

	FFVConditionContext Context;
	Context.WorldContext = GetWorld();
	Finished->OnFinished.Apply(Context);
	OnFinished.Broadcast();
}

void UFVCinematicSubsystem::SetPlayerState(bool bInCinematic) const
{
	if (!Current || !Current->bDisablePlayerInput)
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}
	PC->SetCinematicMode(bInCinematic, Current->bHidePlayer, false, true, true);
}

void UFVCinematicSubsystem::Cleanup()
{
	SetPlayerState(false);
	if (Player)
	{
		Player->OnFinished.RemoveAll(this);
		Player->Stop();
	}
	if (SequenceActor)
	{
		SequenceActor->Destroy();
	}
	Player = nullptr;
	SequenceActor = nullptr;
	Current = nullptr;
}

void FFVEffect_PlayCinematic::Apply(const FFVConditionContext& Context) const
{
	if (UFVCinematicSubsystem* Subsystem = UFVCinematicSubsystem::Get(Context.WorldContext))
	{
		Subsystem->Play(Cinematic);
	}
}

FText FFVEffect_PlayCinematic::GetDescription() const
{
	const FText Name = Cinematic ? Cinematic->Display.Name : LOCTEXT("None", "<none>");
	return FText::Format(LOCTEXT("Play", "Play cinematic {0}"), Name);
}

#undef LOCTEXT_NAMESPACE