#pragma once
#include "StateTreeTaskBase.h"
#include "FVSTT_TriggerFlowGraph.generated.h"

class UFVFlowTriggerComponent;
class UFlowComponent;

USTRUCT(BlueprintType,  Category = "Flicker Void", meta = (DisplayName = "Trigger FlowGraph Instance Data"))
struct FTriggerFlowGraphInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AActor> Actor;
	
	UPROPERTY(EditAnywhere, Category = FlowGraph)
	TObjectPtr<AActor> OtherActor;
	
	UPROPERTY(EditAnywhere, Category = FlowGraph)
	FGameplayTag IdentityTag;
};

USTRUCT(Category = "Flicker Void", meta = (DisplayName = "Trigger FlowGraph"))
struct FFVSTT_TriggerFlowGraph : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FTriggerFlowGraphInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FTriggerFlowGraphInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context, 
		const FStateTreeTransitionResult& Transition) const override;
	
	virtual void ExitState(
		FStateTreeExecutionContext& Context, 
		const FStateTreeTransitionResult& Transition) const override;
	
#if WITH_EDITOR
	virtual FName GetIconName() const override { return FName("GenericPlay"); }
	virtual FColor GetIconColor() const override { return FColor::Silver; }
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
	{
		if (const FTriggerFlowGraphInstanceData* Data = InstanceDataView.GetPtr<FTriggerFlowGraphInstanceData>())
		{
			const FGameplayTag IdentityTag = Data->IdentityTag;

			return FText::Format(
				NSLOCTEXT("FlickerVoid", "TriggerFlowGraphDescription", "Trigger FlowGraph with identity {0}"),
				FText::FromString(IdentityTag.ToString())
			);
		}
		
		return NSLOCTEXT("FlickerVoid", "TriggerFlowGraphTask_NoData", "Trigger FlowGraph(No Identity Tag Defined)");
	}
#endif
};
