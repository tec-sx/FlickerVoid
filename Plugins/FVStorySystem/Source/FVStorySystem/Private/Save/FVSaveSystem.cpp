#include "Save/FVSaveSystem.h"
#include "Quest/FVQuest.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Facts/FVFactDatabase.h"
#include "Save/FVSaveableComponent.h"
#include "FlowSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UFVSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildParticipants();
}

void UFVSaveSubsystem::BuildParticipants()
{
	Participants.Reset();
	for (const TSoftClassPtr<UFVSaveParticipant>& ClassPtr : GetDefault<UFVSaveSettings>()->Participants)
	{
		if (UClass* Class = ClassPtr.LoadSynchronous())
		{
			Participants.Add(NewObject<UFVSaveParticipant>(this, Class));
		}
	}
	Participants.Sort([](const UFVSaveParticipant& A, const UFVSaveParticipant& B) { return A.Order < B.Order; });
}

bool UFVSaveSubsystem::SaveToSlot(const FString& SlotName)
{
	return WriteSlot(SlotName, false);
}

bool UFVSaveSubsystem::SaveCheckpoint()
{
	return WriteSlot(GetDefault<UFVSaveSettings>()->CheckpointSlot, true);
}

bool UFVSaveSubsystem::LoadCheckpoint()
{
	return LoadFromSlot(GetDefault<UFVSaveSettings>()->CheckpointSlot);
}

bool UFVSaveSubsystem::WriteSlot(const FString& SlotName, bool bCheckpoint)
{
	if (!CanSave() || SlotName.IsEmpty())
	{
		return false;
	}

	UFVSaveGame* SaveGame = Cast<UFVSaveGame>(UGameplayStatics::CreateSaveGameObject(UFVSaveGame::StaticClass()));
	SaveGame->SaveSlotName = SlotName;
	WriteMetadata(*SaveGame, SlotName, bCheckpoint);
	WritePipeline(*SaveGame);

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0))
	{
		return false;
	}

	UpdateIndex(SaveGame->Metadata, false);
	OnSaved.Broadcast();
	return true;
}

void UFVSaveSubsystem::WriteMetadata(UFVSaveGame& SaveGame, const FString& SlotName, bool bCheckpoint)
{
	FFVSaveMetadata& Meta = SaveGame.Metadata;
	Meta.Version = UFVSaveGame::CurrentVersion;
	Meta.SlotName = SlotName;
	Meta.Timestamp = FDateTime::Now();
	Meta.bIsCheckpoint = bCheckpoint;
	Meta.ThumbnailPNG = MoveTemp(PendingThumbnail);
	PendingThumbnail.Reset();

	if (const UWorld* World = GetWorld())
	{
		Meta.MapName = UGameplayStatics::GetCurrentLevelName(World);
		Meta.PlayTimeSeconds = World->GetTimeSeconds();
	}
}

void UFVSaveSubsystem::WritePipeline(UFVSaveGame& SaveGame)
{
	if (const UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		SaveGame.Facts = Facts->GetAllFacts();
	}

	if (UFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<UFlowSubsystem>())
	{
		Flow->OnGameSaved(&SaveGame);
	}

	if (const UFVQuestSubsystem* Quests = UFVQuestSubsystem::Get(this))
	{
		SaveGame.TrackedQuests = Quests->GetTrackedQuestPaths();
	}

	SaveGame.Actors.Reset();
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(*It);
			if (Saveable && Saveable->GetSaveId().IsValid())
			{
				Saveable->WriteActorData(SaveGame.Actors.Add(Saveable->GetSaveId()).Data);
			}
		}
	}

	for (UFVSaveParticipant* Participant : Participants)
	{
		Participant->WriteSave(&SaveGame, this);
	}
}

bool UFVSaveSubsystem::LoadFromSlot(const FString& SlotName)
{
	UFVSaveGame* SaveGame = Cast<UFVSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGame)
	{
		return false;
	}

	if (SaveGame->Metadata.Version > UFVSaveGame::CurrentVersion)
	{
		return false;
	}

	ReadPipeline(*SaveGame);
	LastLoaded = SaveGame;
	OnLoaded.Broadcast();
	return true;
}

void UFVSaveSubsystem::ReadPipeline(UFVSaveGame& SaveGame)
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->RestoreFacts(SaveGame.Facts);
	}

	if (UFlowSubsystem* Flow = GetGameInstance()->GetSubsystem<UFlowSubsystem>())
	{
		Flow->OnGameLoaded(&SaveGame);
	}

	if (UFVQuestSubsystem* Quests = UFVQuestSubsystem::Get(this))
	{
		Quests->RestoreTrackedQuests(SaveGame.TrackedQuests);
	}

	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			UFVSaveableComponent* Saveable = UFVSaveableComponent::Find(*It);
			const FFVActorSaveRecord* Record = Saveable ? SaveGame.Actors.Find(Saveable->GetSaveId()) : nullptr;
			if (Record != nullptr)
			{
				Saveable->ReadActorData(Record->Data);
			}
		}
	}

	for (UFVSaveParticipant* Participant : Participants)
	{
		Participant->ReadSave(&SaveGame, this);
	}
}

bool UFVSaveSubsystem::DeleteSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
	{
		return false;
	}

	FFVSaveMetadata Meta;
	Meta.SlotName = SlotName;
	UpdateIndex(Meta, true);
	return true;
}

TArray<FFVSaveMetadata> UFVSaveSubsystem::GetSlots() const
{
	const UFVSlotIndex* Index = LoadIndex();
	return Index ? Index->Slots : TArray<FFVSaveMetadata>();
}

UFVSlotIndex* UFVSaveSubsystem::LoadIndex() const
{
	return Cast<UFVSlotIndex>(UGameplayStatics::LoadGameFromSlot(GetDefault<UFVSaveSettings>()->SlotIndexName, 0));
}

void UFVSaveSubsystem::UpdateIndex(const FFVSaveMetadata& Metadata, bool bRemove)
{
	UFVSlotIndex* Index = LoadIndex();
	if (!Index)
	{
		Index = Cast<UFVSlotIndex>(UGameplayStatics::CreateSaveGameObject(UFVSlotIndex::StaticClass()));
	}

	Index->Slots.RemoveAll([&Metadata](const FFVSaveMetadata& Entry) { return Entry.SlotName == Metadata.SlotName; });
	if (!bRemove)
	{
		FFVSaveMetadata Entry = Metadata;
		Entry.ThumbnailPNG.Reset();
		Index->Slots.Add(Entry);
	}

	UGameplayStatics::SaveGameToSlot(Index, GetDefault<UFVSaveSettings>()->SlotIndexName, 0);
}
