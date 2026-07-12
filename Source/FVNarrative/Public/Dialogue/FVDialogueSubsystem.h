#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Dialogue/FVDialogueContext.h"
#include "Dialogue/Data/FVDialogueLineRow.h"
#include "Dialogue/Conditions/FVDialogueConditionEvaluator.h"
#include "FVDialogueSubsystem.generated.h"

class UFVNarrativeSettings;
class UStateTree;
class UStateTreeComponent;
class UFVDialogueContextBuilder;
class UFVDialogueDatabaseSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnDialogueLineStarted,
    FName,                  LineID,
    const FFVDialogueLineRow&, LineRow,
    const FFVDialogueContext&, Context);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnDialogueResponsesReady,
    const TArray<FFVEvaluatedResponse>&, Responses);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueHideResponses);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueSessionEnded);

USTRUCT(BlueprintType)
struct FFVDialogueSignal
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FGameplayTag SpeakerID;
    
    UPROPERTY(BlueprintReadWrite)
    FGameplayTag Signal;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Speaker;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Listener;
};

UCLASS()
class FLICKERVOIDNARRATIVE_API UFVDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ── Subsystem Lifecycle ───────────────────────────────────────────────────

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Signal Entry Point ────────────────────────────────────────────────────
    
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void BroadcastSignal(const FFVDialogueSignal& Signal, const bool bForceInterrupt = false);
    
    void BroadcastSignal(
        const FGameplayTag& SpeakerID,
        const FGameplayTag& Signal, 
        AActor* Speaker, 
        AActor* Listener, 
        const bool bForceInterrupt = false);

    // ── Player Choice Submission ──────────────────────────────────────────────
    
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void SubmitPlayerChoice(const FName& ResponseID);

    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void SubmitLineDismissed();

    // ── Session Query ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category="Dialogue")
    bool IsSessionActive() const { return bSessionActive; }

    UFUNCTION(BlueprintPure, Category="Dialogue")
    bool IsCurrentLineComplete() const { return bCurrentLineComplete; }

    UFUNCTION(BlueprintPure, Category="Dialogue")
    FName GetPendingPlayerChoice() const { return PendingPlayerChoice; }

    void ClearPendingPlayerChoice() { PendingPlayerChoice = NAME_None; }

    // ── Delegates (UI & Audio bind here) ─────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category="Dialogue|Events")
    FOnDialogueLineStarted OnLineStarted;

    UPROPERTY(BlueprintAssignable, Category="Dialogue|Events")
    FOnDialogueResponsesReady OnResponsesReady;

    UPROPERTY(BlueprintAssignable, Category="Dialogue|Events")
    FOnDialogueHideResponses OnHideResponses;

    UPROPERTY(BlueprintAssignable, Category="Dialogue|Events")
    FOnDialogueSessionEnded OnSessionEnded;

    // ── Configuration ─────────────────────────────────────────────────────────
    
    UFUNCTION(BlueprintPure, Category = "Dialogue|Config")
    const UFVNarrativeSettings* GetConfig() const { return  DialogueSettings.Get(); }
 
    const UDataTable* GetResponseTable() const;

    UFUNCTION(BlueprintCallable , Category="Dialogue")
    void BroadcastLineStarted(FName LineID, const FFVDialogueLineRow& Row, const FFVDialogueContext& Context);
    
    UFUNCTION(BlueprintCallable , Category="Dialogue")
    void BroadcastResponseOptions(const TArray<FFVEvaluatedResponse>& Responses);
    
    UFUNCTION(BlueprintCallable , Category="Dialogue")
    void BroadcastHideResponses();
    
    UFUNCTION(BlueprintCallable , Category="Dialogue")
    void BroadcastSessionEnded();

    void PersistConsumedLines(const TSet<FName>& NewlyConsumed);
    void NotifySessionEnded(const FFVDialogueContext& EndedContext);

private:
    // ── Active Session State ──────────────────────────────────────────────────

    bool bSessionActive        = false;
    bool bCurrentLineComplete  = false;
    FName PendingPlayerChoice  = NAME_None;
    
    FFVDialogueContext ActiveContext;
    TSet<FName> PersistentConsumedLineIDs;
    
    UPROPERTY()
    TObjectPtr<UFVNarrativeSettings> DialogueSettings;

    // ── Helpers ───────────────────────────────────────────────────────────────
    
    void InitializeConfig(UFVNarrativeSettings* InConfig);
    void StartSession(const FFVDialogueContext& Context);
    void EndCurrentSession();
    void LoadConsumedLinesFromSave();
    void SaveConsumedLinesToSave();

    /** Lazily creates and returns the context builder instance. */
    UFVDialogueContextBuilder* GetOrCreateContextBuilder();

    UPROPERTY()
    TObjectPtr<UFVDialogueContextBuilder> ContextBuilderInstance;
};