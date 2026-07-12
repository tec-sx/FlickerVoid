
#include "Dialogue/FVDialogueDatabaseSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("UFVDialogueDatabaseSubsystem: Initialized."));
}

void UFVDialogueDatabaseSubsystem::Deinitialize()
{
    // Release all async load handles
    PendingLoads.Empty();
    LineIndex.Empty();
    LineFlatMap.Empty();
    IndexedTables.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueDatabaseSubsystem::RegisterLineTable(const FFVDialogueTableRegistration& Registration)
{
    if (Registration.LineTable.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("UFVDialogueDatabaseSubsystem::RegisterLineTable — null table reference."));
        return;
    }
    
    if (UDataTable* Table = Registration.LineTable.Get())
    {
        if (!IndexedTables.Contains(Table))
        {
            IndexTable(Table);
        }
        
        return;
    }
    
    FStreamableManager& Manager = UAssetManager::GetStreamableManager();
    const TSharedPtr<FStreamableHandle> Handle = Manager.RequestAsyncLoad(
        Registration.LineTable.ToSoftObjectPath(),
        [this, Registration]()
        {
            UDataTable* Table = Registration.LineTable.Get();
            if (Table && !IndexedTables.Contains(Table))
            {
                IndexTable(Table);
            }
        });

    if (Handle.IsValid())
    {
        PendingLoads.Add(Handle);
    }
}

void UFVDialogueDatabaseSubsystem::RegisterResponseTable(UDataTable* Table)
{
    if (!Table)
    {
        UE_LOG(LogTemp, Warning, TEXT("UFVDialogueDatabaseSubsystem::RegisterResponseTable — null table."));
        return;
    }
    ResponseTable = Table;
    UE_LOG(LogTemp, Log, TEXT("UFVDialogueDatabaseSubsystem: Response table registered (%s)."), *Table->GetName());
}

void UFVDialogueDatabaseSubsystem::UnregisterLineTable(UDataTable* Table)
{
    if (!Table) return;
    RemoveTableFromIndex(Table);
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueDatabaseSubsystem::IndexTable(UDataTable* Table)
{
    if (!Table) return;

    IndexedTables.Add(Table);
    int32 IndexedCount = 0;

    Table->ForeachRow<FFVDialogueLineRow>(
        TEXT("IndexTable"),
        [this, &IndexedCount](const FName& RowKey, const FFVDialogueLineRow& Row)
        {
            if (!Row.Signal.IsValid() || !Row.SpeakerID.IsValid())
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("DialogueDB: Row '%s' has invalid SignalTag or SpeakerID — skipped."),
                    *RowKey.ToString());
                return;
            }

            const FFVDialogueIndexKey IndexKey{ Row.Signal, Row.SpeakerID };
            TArray<TPair<FName, const FFVDialogueLineRow*>>& Bucket = LineIndex.FindOrAdd(IndexKey);
            Bucket.Add(TPair<FName, const FFVDialogueLineRow*>(RowKey, &Row));

            LineFlatMap.Add(RowKey, &Row);
            ++IndexedCount;
        });

    UE_LOG(LogTemp, Log, TEXT("DialogueDB: Indexed %d rows from table '%s'."),
        IndexedCount, *Table->GetName());
}

void UFVDialogueDatabaseSubsystem::RemoveTableFromIndex(UDataTable* Table)
{
    if (!Table || !IndexedTables.Contains(Table)) return;
    
    TSet<FName> TableRowKeys;
    Table->ForeachRow<FFVDialogueLineRow>(TEXT("RemoveTableFromIndex"),
        [&TableRowKeys](const FName& RowKey, const FFVDialogueLineRow&)
        {
            TableRowKeys.Add(RowKey);
        });
    
    for (const FName& Key : TableRowKeys)
    {
        LineFlatMap.Remove(Key);
    }
    
    for (auto& Pair : LineIndex)
    {
        Pair.Value.RemoveAll([&TableRowKeys](const TPair<FName, const FFVDialogueLineRow*>& Entry)
        {
            return TableRowKeys.Contains(Entry.Key);
        });
    }
    
    for (auto It = LineIndex.CreateIterator(); It; ++It)
    {
        if (It.Value().IsEmpty()) It.RemoveCurrent();
    }

    IndexedTables.Remove(Table);

    UE_LOG(LogTemp, Log, TEXT("DialogueDB: Unregistered table '%s' (%d rows removed)."),
        *Table->GetName(), TableRowKeys.Num());
}

// ─────────────────────────────────────────────────────────────────────────────

bool UFVDialogueDatabaseSubsystem::QueryLines(
    const FGameplayTag& Signal,
    const FGameplayTag& SpeakerID,
    TArray<TPair<FName, const FFVDialogueLineRow*>>& OutCandidates) const
{
    OutCandidates.Reset();

    const FFVDialogueIndexKey Key{ Signal, SpeakerID };
    if (const TArray<TPair<FName, const FFVDialogueLineRow*>>* Bucket = LineIndex.Find(Key))
    {
        OutCandidates = *Bucket;
        return !OutCandidates.IsEmpty();
    }

    return false;
}

FFVDialogueResponseRow UFVDialogueDatabaseSubsystem::FindResponse(const FName& ResponseID) const
{
    FFVDialogueResponseRow Result;
    
    if (ResponseTable.IsValid())
    {
        if (auto ResponsePtr = FindResponsePtr(ResponseID))
        {
            Result = *ResponsePtr;
        }
    }
    
    return Result;
}

FFVDialogueLineRow UFVDialogueDatabaseSubsystem::FindLine(const FName& LineID) const
{
    FFVDialogueLineRow Result;
    
    if (const FFVDialogueLineRow* Found = FindLinePtr(LineID))
    {
        Result = *Found;
    }
    
    return Result;
}

const FFVDialogueResponseRow* UFVDialogueDatabaseSubsystem::FindResponsePtr(const FName& ResponseID) const
{
    if (ResponseTable.IsValid())
    {
        return ResponseTable->FindRow<FFVDialogueResponseRow>(ResponseID, TEXT("FindResponse"));
    }
    
    return nullptr;
}

const FFVDialogueLineRow* UFVDialogueDatabaseSubsystem::FindLinePtr(const FName& LineID) const
{
    if (const FFVDialogueLineRow* const* Found = LineFlatMap.Find(LineID))
    {
        return *Found;
    }
    
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Debug
// ─────────────────────────────────────────────────────────────────────────────

int32 UFVDialogueDatabaseSubsystem::GetIndexedLineCount() const
{
    return LineFlatMap.Num();
}

void UFVDialogueDatabaseSubsystem::DumpIndex() const
{
    UE_LOG(LogTemp, Log, TEXT("=== DialogueDB Index Dump ==="));
    UE_LOG(LogTemp, Log, TEXT("Total lines: %d"), LineFlatMap.Num());
    UE_LOG(LogTemp, Log, TEXT("Index buckets:"));

    for (const auto& Pair : LineIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("  [%s + %s] → %d lines"),
            *Pair.Key.Signal.ToString(),
            *Pair.Key.SpeakerID.ToString(),
            Pair.Value.Num());
    }
}