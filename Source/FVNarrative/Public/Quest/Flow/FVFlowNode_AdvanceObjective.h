#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_AdvanceObjective.generated.h"

/**
 * State of a single quest objective, stored as the value of
 * Fact.Quest.<ChapterId>.<QuestId>.Objective.<ObjectiveId>.
 *
 * An objective that was never issued has no fact defined at all, which is what
 * distinguishes "not relevant yet" from "issued but not done".
 *
 * Distinct from EFVObjectiveState in the legacy quest system, which models a
 * richer runtime state machine rather than a fact value.
 */
UENUM(BlueprintType)
enum class EFVObjectiveFactState : uint8
{
	// Issue the objective: defined with value 0.
	Activate = 0,
	// Mark the objective satisfied: value 1.
	Complete = 1
};

/**
 * Issues or completes a quest objective.
 *
 * The most repeated write in any quest graph, so it exists as a node rather than
 * a Change Fact with a hand typed tag.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Advance Objective"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_AdvanceObjective : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_AdvanceObjective();

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName ChapterId;

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName QuestId;

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName ObjectiveId;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVObjectiveFactState NewState = EFVObjectiveFactState::Activate;

private:
	FGameplayTag MakeObjectiveTag() const;
};
