#include "Dialogue/FVDialogueSubsystem.h"
#include "Dialogue/FVDialogueDatabaseSubsystem.h"
#include "Dialogue/FVDialogueContextBuilder.h"
#include "Components/StateTreeComponent.h"
#include "StateTree.h"
#include "FVNarrativeSettings.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    Collection.InitializeDependency<UFVDialogueDatabaseSubsystem>();

    UE_LOG(
        LogTemp, 
        Log, 
        TEXT("UFVDialogueSubsystem: Initialized. %d consumed lines loaded."), PersistentConsumedLineIDs.Num());
    
    const UFVNarrativeSettings* SettingsCDO = GetDefault<UFVNarrativeSettings>();
    if (!SettingsCDO)
    {
        UE_LOG(LogTemp, Error, TEXT("UFVDialogueSettings CDO not found. DialogueSubsystem cannot initialize."));

        return;
    }

    InitializeConfig(const_cast<UFVNarrativeSettings*>(SettingsCDO));
}

void UFVDialogueSubsystem::InitializeConfig(UFVNarrativeSettings* InConfig)
{
    if (!InConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("InitializeConfig called with null settings."));
        return;
    }

    DialogueSettings = InConfig;
    
    if (InConfig->ResponseTable.IsNull())
    {
        if (UDataTable* ResponseTable = InConfig->ResponseTable.LoadSynchronous())
        {
            if (UFVDialogueDatabaseSubsystem* DB = GetGameInstance()->GetSubsystem<UFVDialogueDatabaseSubsystem>())
            {
                DB->RegisterResponseTable(ResponseTable);
                UE_LOG(
                    LogTemp, 
                    Log, 
                    TEXT("Registered Dialogue response table: %s"), *ResponseTable->GetPathName());
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue response table not set in project settings."));
    }

    if (UFVDialogueDatabaseSubsystem* DB = GetGameInstance()->GetSubsystem<UFVDialogueDatabaseSubsystem>())
    {
        for (const TSoftObjectPtr<UDataTable>& SoftTable : InConfig->GlobalLineTables)
        {
            UDataTable* Table = SoftTable.Get();
            if (!Table && SoftTable.ToSoftObjectPath().IsValid())
            {
                // Small tables: load synchronously so they are available immediately.
                UObject* Loaded = SoftTable.ToSoftObjectPath().TryLoad();
                Table = Cast<UDataTable>(Loaded);
            }

            if (Table)
            {
                FFVDialogueTableRegistration Reg;
                Reg.LineTable = Table;
                DB->RegisterLineTable(Reg);
                UE_LOG(LogTemp, Log, TEXT("Registered global dialogue line table: %s"), *Table->GetPathName());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to resolve global dialogue table: %s"), *SoftTable.ToString());
            }
        }
    }
    
    LoadConsumedLinesFromSave();

    UE_LOG(LogTemp, Log, TEXT("DialogueSubsystem initialized with settings CDO: %s"), *InConfig->GetPathName());
}

void UFVDialogueSubsystem::Deinitialize()
{
    EndCurrentSession();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::BroadcastSignal(const FFVDialogueSignal& Signal, const bool bForceInterrupt)
{
    BroadcastSignal(
        Signal.SpeakerID,
        Signal.Signal, 
        Signal.Speaker.Get(),
        Signal.Listener.Get(),
        bForceInterrupt);
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::BroadcastSignal(
    const FGameplayTag& SpeakerID,
    const FGameplayTag& Signal,
    AActor* Speaker,
    AActor* Listener,
    const bool bForceInterrupt)
{
    if (!Signal.IsValid() || !Speaker || !Listener)
    {
        UE_LOG(LogTemp, Warning, TEXT("UFVDialogueSubsystem::BroadcastSignal — invalid parameters."));
        return;
    }

    // Existing session handling
    if (bSessionActive)
    {
        if (!bForceInterrupt)
        {
            UE_LOG(LogTemp, Verbose,
                TEXT("DialogueSubsystem: Signal '%s' ignored — session already active."),
                *Signal.ToString());
            return;
        }
        EndCurrentSession();
    }

    // Build context
    const UFVDialogueContextBuilder* Builder = GetOrCreateContextBuilder();
    const FFVDialogueContext Context = Builder->Build(
        SpeakerID, 
        Signal, 
        Speaker, 
        Listener, 
        PersistentConsumedLineIDs);

    if (!Context.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("DialogueSubsystem: Context build failed for signal '%s'."), *Signal.ToString());
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("DialogueSubsystem: Starting session for signal '%s' speaker '%s'."),
        *Signal.ToString(), *Context.SpeakerID.ToString());

    StartSession(Context);
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::StartSession(const FFVDialogueContext& Context)
{
    bSessionActive       = true;
    bCurrentLineComplete = false;
    PendingPlayerChoice  = NAME_None;
    ActiveContext        = Context;

    // Create a transient actor to host the State Tree component.
    // This actor lives only for the duration of the dialogue session.
    UWorld* World = GetGameInstance()->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueSubsystem::StartSession — no world."));
        bSessionActive = false;
        return;
    }

    AActor* HostActor = World->SpawnActor<AActor>();
    if (!HostActor)
    {
        bSessionActive = false;
        return;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::EndCurrentSession()
{
    if (!bSessionActive) return;

    bSessionActive       = false;
    bCurrentLineComplete = false;
    PendingPlayerChoice  = NAME_None;
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::SubmitPlayerChoice(const FName& ResponseID)
{
    if (!bSessionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueSubsystem::SubmitPlayerChoice — no active session."));
        return;
    }
    PendingPlayerChoice = ResponseID;
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::SubmitLineDismissed()
{
    bCurrentLineComplete = true;
}

// ─────────────────────────────────────────────────────────────────────────────

const UDataTable* UFVDialogueSubsystem::GetResponseTable() const
{
    return DialogueSettings ? DialogueSettings->ResponseTable.Get() : nullptr;
}

void UFVDialogueSubsystem::BroadcastLineStarted(
    FName LineID, 
    const FFVDialogueLineRow& Row, 
    const FFVDialogueContext& Context)
{
    bCurrentLineComplete = false;
    OnLineStarted.Broadcast(LineID, Row, Context);
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::BroadcastResponseOptions(const TArray<FFVEvaluatedResponse>& Responses)
{
    OnResponsesReady.Broadcast(Responses);
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::BroadcastHideResponses()
{
    OnHideResponses.Broadcast();
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::BroadcastSessionEnded()
{
    OnSessionEnded.Broadcast();
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::PersistConsumedLines(const TSet<FName>& NewlyConsumed)
{
    for (const FName& LineID : NewlyConsumed)
    {
        PersistentConsumedLineIDs.Add(LineID);
    }
    SaveConsumedLinesToSave();
}

// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::NotifySessionEnded(const FFVDialogueContext& EndedContext)
{
    UE_LOG(LogTemp, Log,
        TEXT("DialogueSubsystem: Session ended for speaker '%s'."),
        *EndedContext.SpeakerID.ToString());

    EndCurrentSession();
}

// ─────────────────────────────────────────────────────────────────────────────
// Persistence
// ─────────────────────────────────────────────────────────────────────────────

void UFVDialogueSubsystem::LoadConsumedLinesFromSave()
{
    // TODO: Integrate with your save game system.
    // Example:
    //   if (UYourSaveGame* Save = Cast<UYourSaveGame>(
    //           UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot"), 0)))
    //   {
    //       PersistentConsumedLineIDs = Save->ConsumedDialogueLineIDs;
    //   }
}

void UFVDialogueSubsystem::SaveConsumedLinesToSave()
{
    // TODO: Integrate with your save game system.
    // Example:
    //   if (UYourSaveGame* Save = GetOrCreateSaveGame())
    //   {
    //       Save->ConsumedDialogueLineIDs = PersistentConsumedLineIDs;
    //       UGameplayStatics::SaveGameToSlot(Save, TEXT("SaveSlot"), 0);
    //   }
}

// ─────────────────────────────────────────────────────────────────────────────

UFVDialogueContextBuilder* UFVDialogueSubsystem::GetOrCreateContextBuilder()
{
    if (ContextBuilderInstance)
    {
        return ContextBuilderInstance;
    }

    UFVNarrativeSettings* Settings = GetMutableDefault<UFVNarrativeSettings>();
    TSubclassOf<UFVDialogueContextBuilder> BuilderClass = UFVDialogueContextBuilder::StaticClass();

    if (Settings && !Settings->ContextBuilderClass.IsNull())
    {
        if (UClass* LoadedClass = Settings->ContextBuilderClass.LoadSynchronous())
        {
            if (LoadedClass->IsChildOf(UFVDialogueContextBuilder::StaticClass()))
            {
                BuilderClass = LoadedClass;
            }
        }
    }

    ContextBuilderInstance = NewObject<UFVDialogueContextBuilder>(this, BuilderClass);
    return ContextBuilderInstance;
}