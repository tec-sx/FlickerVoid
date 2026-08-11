#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "Quest/FVQuestFactHelpers.h"
#include "FVFlowNode_SetChapterStage.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Set Chapter Stage"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_SetChapterStage : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_SetChapterStage();

	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(
		const UFlowNodeAddOn* AddOnTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Chapter", meta = (Categories = "Fact.Chapter"))
	FGameplayTag ChapterStage;

	UPROPERTY(EditAnywhere, Category = "Chapter")
	EFVQuestStage Stage = EFVQuestStage::InProgress;
};
