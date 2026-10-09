#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVQuest.generated.h"

class UFVFactDatabase;
class UFlowAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnQuestsChanged);

UENUM(BlueprintType)
enum class EFVQuestState : uint8
{
	NotStarted,
	Active,
	Completed,
	Failed
};

USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVQuestObjective
{
	GENERATED_BODY()

	/** Fact set to 1 when this objective completes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Fact"))
	FGameplayTag Fact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Description;

	/** Evaluated whenever facts change while the quest is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FFVConditionSet CompleteWhen;

	/** Objective is shown/evaluated only when these pass (empty = always). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FFVConditionSet VisibleWhen;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bOptional = false;
};

/** A quest. Id is the fact holding EFVQuestState. */
UCLASS(BlueprintType)
class FVSTORYSYSTEM_API UFVQuestDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** When not started and these pass, the quest starts automatically (empty = manual start only). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FFVConditionSet AutoStartWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	TArray<FFVQuestObjective> Objectives;

	/** While active, the quest fails when these pass (empty = never). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FFVConditionSet FailWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FFVEffectList OnStarted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FFVEffectList OnCompleted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	FFVEffectList OnFailed;

	/** Optional graph that runs while the quest is active (owned by the quest subsystem). Restarts from its start after loading. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UFlowAsset> Flow;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

protected:
	virtual bool RequiresId() const override { return true; }
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Quests"))
class FVSTORYSYSTEM_API UFVQuestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	/** Quests tracked by the quest subsystem (auto-start, objective evaluation). */
	UPROPERTY(Config, EditAnywhere, Category = "Quests")
	TArray<TSoftObjectPtr<UFVQuestDefinition>> Quests;
};

UCLASS()
class FVSTORYSYSTEM_API UFVQuestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVQuestSubsystem* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintPure, Category = "Quest")
	EFVQuestState GetState(const UFVQuestDefinition* Quest) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsObjectiveComplete(const FFVQuestObjective& Objective) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsObjectiveVisible(const FFVQuestObjective& Objective) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuest(UFVQuestDefinition* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(UFVQuestDefinition* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FailQuest(UFVQuestDefinition* Quest);

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UFVQuestDefinition*> GetQuests(EFVQuestState State) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	UFVQuestDefinition* GetLastChangedQuest() const { return LastChanged; }

	/** Fired on start/complete/fail and on objective completion. Read GetLastChangedQuest. */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FFVOnQuestsChanged OnQuestsChanged;

	/** Re-evaluates auto-start, objectives and failure for all tracked quests. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void Evaluate();

	TArray<FSoftObjectPath> GetTrackedQuestPaths() const;

	/** Re-tracks quests from a save and re-evaluates. Quest states themselves live in facts. */
	void RestoreTrackedQuests(const TArray<FSoftObjectPath>& Paths);

private:
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);
	bool SetState(UFVQuestDefinition* Quest, EFVQuestState NewState, EFVQuestState RequiredState);
	void EvaluateQuest(UFVQuestDefinition* Quest);
	bool UpdateObjectives(const UFVQuestDefinition* Quest);
	bool AreRequiredObjectivesDone(const UFVQuestDefinition* Quest) const;
	void Notify(UFVQuestDefinition* Quest);
	void StartQuestFlow(UFVQuestDefinition* Quest);
	void StopQuestFlow(UFVQuestDefinition* Quest, bool bAbort);
	void StartActiveQuestFlows();
	FFVConditionContext MakeContext() const;
	UFVFactDatabase* GetFacts() const;

	UPROPERTY()
	TArray<TObjectPtr<UFVQuestDefinition>> Quests;

	UPROPERTY()
	TObjectPtr<UFVQuestDefinition> LastChanged;

	FDelegateHandle FactHandle;
	bool bEvaluating = false;
	bool bDirty = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Quest State"))
struct FVSTORYSYSTEM_API FFVCondition_QuestState : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVQuestDefinition> Quest;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVQuestState State = EFVQuestState::Active;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Set Quest State"))
struct FVSTORYSYSTEM_API FFVEffect_SetQuestState : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVQuestDefinition> Quest;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (InvalidEnumValues = "NotStarted"))
	EFVQuestState State = EFVQuestState::Active;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};