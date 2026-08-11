#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_AdvanceObjective.generated.h"

UENUM(BlueprintType)
enum class EFVObjectiveFactState : uint8
{
	Activate = 0,
	Complete = 1
};

UCLASS(NotBlueprintable, meta = (DisplayName = "Advance Objective"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_AdvanceObjective : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_AdvanceObjective();

	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(
		const UFlowNodeAddOn* AddOnTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Quest", meta = (Categories = "Fact.Quest"))
	FGameplayTag Objective;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVObjectiveFactState NewState = EFVObjectiveFactState::Activate;
};
