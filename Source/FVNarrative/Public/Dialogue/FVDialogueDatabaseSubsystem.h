#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Dialogue/Data/FVDialogueLineRow.h"
#include "Dialogue/Data/FVDialogueResponseRow.h"
#include "FVDialogueDatabaseSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FFVDialogueTableRegistration
{
	GENERATED_BODY()
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(RowType="/Script/FlickerVoid.FVDialogueLineRow"))
	TSoftObjectPtr<UDataTable> LineTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories = "Dialogue.Speaker"))
	FGameplayTagContainer RestrictedToSpeakerIDs;
};

// ─────────────────────────────────────────────────────────────────────────────

struct FFVDialogueIndexKey
{
	FGameplayTag Signal;
	FGameplayTag SpeakerID;
 
	bool operator==(const FFVDialogueIndexKey& Other) const
	{
		return Signal == Other.Signal && SpeakerID == Other.SpeakerID;
	}
};

inline uint32 GetTypeHash(const FFVDialogueIndexKey& Key)
{
	// GetTypeHash for FGameplayTag already exists; combine the two hashes.
	const uint32 HashA = GetTypeHash(Key.Signal);
	const uint32 HashB = GetTypeHash(Key.SpeakerID);

	// A simple good-quality combine (boost-like)
	return HashA ^ (HashB + 0x9e3779b9u + (HashA << 6) + (HashA >> 2));
}

// ─────────────────────────────────────────────────────────────────────────────
// UFVDialogueDatabaseSubsystem
// Responsibilities:
//   1. Accept DataTable registrations (from NPCs loading in, data assets, etc.)
//   2. Index all rows by (SignalTag, SpeakerID) for O(1) query dispatch
//   3. Serve FDialogueLineRow arrays to UDialogueSubsystem on request
//   4. Serve FDialogueResponseRow lookups by ID
//
// DataTables are loaded async the first time they're registered; subsequent
// queries are pure in-memory lookups.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class FLICKERVOIDNARRATIVE_API UFVDialogueDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // ── Subsystem Lifecycle ───────────────────────────────────────────────────
 
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
 
    // ── Registration ──────────────────────────────────────────────────────────
	
    UFUNCTION(BlueprintCallable, Category="Dialogue|Database")
    void RegisterLineTable(const FFVDialogueTableRegistration& Registration);
	
    UFUNCTION(
    	BlueprintCallable, 
    	Category="Dialogue|Database",
    	meta=(RowType="/Script/FlickerVoid.FVDialogueResponseRow"))
    void RegisterResponseTable(UDataTable* ResponseTable);
	
    UFUNCTION(BlueprintCallable, Category="Dialogue|Database")
    void UnregisterLineTable(UDataTable* Table);
 
    // ── Query ─────────────────────────────────────────────────────────────────
    
	bool QueryLines(
        const FGameplayTag& Signal,
        const FGameplayTag& SpeakerID,
        TArray<TPair<FName, const FFVDialogueLineRow*>>& OutCandidates) const;
	
	UFUNCTION(BlueprintCallable, Category="Dialogue|Database")
    FFVDialogueResponseRow FindResponse(const FName& ResponseID) const;
    
	UFUNCTION(BlueprintCallable, Category="Dialogue|Database")
	FFVDialogueLineRow FindLine(const FName& LineID) const;
	
	const FFVDialogueResponseRow* FindResponsePtr(const FName& ResponseID) const;
	const FFVDialogueLineRow* FindLinePtr(const FName& LineID) const;
	
    // ── Debug ─────────────────────────────────────────────────────────────────
 
    UFUNCTION(BlueprintCallable, Category="Dialogue|Debug")
    int32 GetIndexedLineCount() const;
 
    UFUNCTION(BlueprintCallable, Category="Dialogue|Debug")
    void DumpIndex() const;
 
private:
    TMap<FFVDialogueIndexKey, TArray<TPair<FName, const FFVDialogueLineRow*>>> LineIndex;
    TMap<FName, const FFVDialogueLineRow*> LineFlatMap;
    TSet<TWeakObjectPtr<UDataTable>> IndexedTables;
    TWeakObjectPtr<UDataTable> ResponseTable;

    void IndexTable(UDataTable* Table);
    void RemoveTableFromIndex(UDataTable* Table);
	
    TArray<TSharedPtr<struct FStreamableHandle>> PendingLoads;
};
