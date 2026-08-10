#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "Quest/FVQuestFactHelpers.h"
#include "FVFlowNode_SetQuestStage.generated.h"

/**
 * Writes Fact.Quest.<ChapterId>.<QuestId>.Stage, or Fact.Chapter.<ChapterId>.Stage
 * when Quest Id is left empty.
 *
 * Exists so graphs never contain raw stage integers.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Set Quest Stage"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_SetQuestStage : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_SetQuestStage();

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName ChapterId;

	/** Leave empty to write the chapter stage instead of a quest stage. */
	UPROPERTY(EditAnywhere, Category = "Quest")
	FName QuestId;

	UPROPERTY(EditAnywhere, Category = "Quest")
	EFVQuestStage Stage = EFVQuestStage::Accepted;

private:
	FGameplayTag MakeStageTag() const;
};
