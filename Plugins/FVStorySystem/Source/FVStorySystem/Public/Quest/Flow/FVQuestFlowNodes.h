#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "Interfaces/FlowPredicateInterface.h"
#include "Nodes/FlowNode.h"
#include "Quest/FVQuest.h"
#include "FVQuestFlowNodes.generated.h"

/** Flow node: moves a quest to Active / Completed / Failed. Replaces Set Quest Stage / Set Chapter Stage. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Set Quest State"))
class FVSTORYSYSTEM_API UFVFlowNode_SetQuestState : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_SetQuestState();

	UPROPERTY(EditAnywhere, Category = "Quest")
	TObjectPtr<UFVQuestDefinition> Quest;

	UPROPERTY(EditAnywhere, Category = "Quest", meta = (InvalidEnumValues = "NotStarted"))
	EFVQuestState State = EFVQuestState::Active;

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

private:
	bool ApplyState(UFVQuestSubsystem& Subsystem) const;
};

/** Flow node: marks an objective fact as done (fact = 1). Replaces Advance Objective. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Complete Objective"))
class FVSTORYSYSTEM_API UFVFlowNode_CompleteObjective : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_CompleteObjective();

	UPROPERTY(EditAnywhere, Category = "Quest", meta = (Categories = "Fact"))
	FGameplayTag ObjectiveFact;

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};

/** Latent flow node: waits until a quest reaches the given state. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Wait For Quest State"))
class FVSTORYSYSTEM_API UFVFlowNode_WaitForQuestState : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_WaitForQuestState();

	UPROPERTY(EditAnywhere, Category = "Quest")
	TObjectPtr<UFVQuestDefinition> Quest;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVQuestState State = EFVQuestState::Completed;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif

private:
	UFUNCTION()
	void HandleQuestsChanged();

	bool TryFinish();

	TWeakObjectPtr<UFVQuestSubsystem> BoundSubsystem;
};

/** Predicate add-on: quest is in the given state. Replaces Quest/Chapter Stage Predicate. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Quest State Predicate"))
class FVSTORYSYSTEM_API UFVFlowNodeAddOn_QuestStatePredicate : public UFlowNodeAddOn, public IFlowPredicateInterface
{
	GENERATED_BODY()

public:
	UFVFlowNodeAddOn_QuestStatePredicate();

	UPROPERTY(EditAnywhere, Category = "Quest")
	TObjectPtr<UFVQuestDefinition> Quest;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVQuestState State = EFVQuestState::Active;

	virtual bool EvaluatePredicate_Implementation() const override;
};
