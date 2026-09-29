#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "Quest/FVQuestFactHelpers.h"
#include "FVFlowNode_SetQuestStage.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Set Quest Stage"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_SetQuestStage : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_SetQuestStage();

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
	FGameplayTag QuestStage;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVQuestStage Stage = EFVQuestStage::Accepted;
};
