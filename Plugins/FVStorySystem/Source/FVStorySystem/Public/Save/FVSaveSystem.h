#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FlowSave.h"
#include "GameplayTagContainer.h"
#include "FVCoreNames.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FVSaveSystem.generated.h"

USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVSaveMetadata
{
GENERATED_BODY()

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
int32 Version = 0;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
FString SlotName;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
FDateTime Timestamp;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
FString MapName;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
float PlayTimeSeconds = 0.f;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
bool bIsCheckpoint = false;

UPROPERTY(SaveGame)
TArray<uint8> ThumbnailPNG;
};

/** SaveGame properties of one placed actor and its components, written by its UFVSaveableComponent. */
USTRUCT()
struct FVSTORYSYSTEM_API FFVActorSaveRecord
{
GENERATED_BODY()

UPROPERTY(SaveGame)
TArray<uint8> Data;
};

UCLASS(BlueprintType)
class FVSTORYSYSTEM_API UFVSaveGame : public UFlowSaveGame
{
GENERATED_BODY()

public:
static constexpr int32 CurrentVersion = 1;

UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Save")
FFVSaveMetadata Metadata;

UPROPERTY(SaveGame)
TMap<FGameplayTag, int32> Facts;

UPROPERTY(SaveGame)
TMap<FName, FString> CustomData;

UPROPERTY(SaveGame)
TArray<FSoftObjectPath> TrackedQuests;

/** Keyed by UFVSaveableComponent::GetSaveId. */
UPROPERTY(SaveGame)
TMap<FGuid, FFVActorSaveRecord> Actors;
};

UCLASS(Abstract, Blueprintable, EditInlineNew)
class FVSTORYSYSTEM_API UFVSaveParticipant : public UObject
{
GENERATED_BODY()

public:
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
int32 Order = 0;

UFUNCTION(BlueprintNativeEvent, Category = "Save")
void WriteSave(UFVSaveGame* SaveGame, UObject* WorldContext);
virtual void WriteSave_Implementation(UFVSaveGame* SaveGame, UObject* WorldContext) {}

UFUNCTION(BlueprintNativeEvent, Category = "Save")
void ReadSave(UFVSaveGame* SaveGame, UObject* WorldContext);
virtual void ReadSave_Implementation(UFVSaveGame* SaveGame, UObject* WorldContext) {}
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Save"))
class FVSTORYSYSTEM_API UFVSaveSettings : public UDeveloperSettings
{
GENERATED_BODY()

public:
virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

UPROPERTY(Config, EditAnywhere, Category = "Save")
FString CheckpointSlot = TEXT("Checkpoint");

UPROPERTY(Config, EditAnywhere, Category = "Save")
FString SlotIndexName = TEXT("FV_SlotIndex");

UPROPERTY(Config, EditAnywhere, Category = "Save")
TArray<TSoftClassPtr<UFVSaveParticipant>> Participants;
};

UCLASS()
class FVSTORYSYSTEM_API UFVSlotIndex : public USaveGame
{
GENERATED_BODY()

public:
UPROPERTY(SaveGame)
TArray<FFVSaveMetadata> Slots;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnSaveEvent);

UCLASS()
class FVSTORYSYSTEM_API UFVSaveSubsystem : public UGameInstanceSubsystem
{
GENERATED_BODY()

public:
virtual void Initialize(FSubsystemCollectionBase& Collection) override;

UFUNCTION(BlueprintCallable, Category = "FV|Save")
bool SaveToSlot(const FString& SlotName);

UFUNCTION(BlueprintCallable, Category = "FV|Save")
bool LoadFromSlot(const FString& SlotName);

UFUNCTION(BlueprintCallable, Category = "FV|Save")
bool SaveCheckpoint();

UFUNCTION(BlueprintCallable, Category = "FV|Save")
bool LoadCheckpoint();

UFUNCTION(BlueprintCallable, Category = "FV|Save")
bool DeleteSlot(const FString& SlotName);

UFUNCTION(BlueprintPure, Category = "FV|Save")
TArray<FFVSaveMetadata> GetSlots() const;

UFUNCTION(BlueprintCallable, Category = "FV|Save")
void AddSaveBlock(FGameplayTag Reason) { SaveBlocks.AddTag(Reason); }

UFUNCTION(BlueprintCallable, Category = "FV|Save")
void RemoveSaveBlock(FGameplayTag Reason) { SaveBlocks.RemoveTag(Reason); }

UFUNCTION(BlueprintPure, Category = "FV|Save")
bool CanSave() const { return SaveBlocks.IsEmpty(); }

UFUNCTION(BlueprintCallable, Category = "FV|Save")
void SetPendingThumbnail(const TArray<uint8>& PNG) { PendingThumbnail = PNG; }

UFUNCTION(BlueprintPure, Category = "FV|Save")
UFVSaveGame* GetLastLoaded() const { return LastLoaded; }

UPROPERTY(BlueprintAssignable, Category = "FV|Save")
FFVOnSaveEvent OnSaved;

UPROPERTY(BlueprintAssignable, Category = "FV|Save")
FFVOnSaveEvent OnLoaded;

private:
bool WriteSlot(const FString& SlotName, bool bCheckpoint);
void WriteMetadata(UFVSaveGame& SaveGame, const FString& SlotName, bool bCheckpoint);
void WritePipeline(UFVSaveGame& SaveGame);
void ReadPipeline(UFVSaveGame& SaveGame);
void UpdateIndex(const FFVSaveMetadata& Metadata, bool bRemove);
UFVSlotIndex* LoadIndex() const;
void BuildParticipants();

UPROPERTY(Transient)
TArray<TObjectPtr<UFVSaveParticipant>> Participants;

UPROPERTY(Transient)
TObjectPtr<UFVSaveGame> LastLoaded;

FGameplayTagContainer SaveBlocks;
TArray<uint8> PendingThumbnail;
};