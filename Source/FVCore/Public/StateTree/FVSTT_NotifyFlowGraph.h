#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "FVSTT_NotifyFlowGraph.generated.h"

USTRUCT(BlueprintType,  Category = "Flicker Void", meta = (DisplayName = "Notify FlowGraph Instance Data"))
struct FNotifyFlowGraphInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	TObjectPtr<AActor> Actor;
	
	UPROPERTY(EditAnywhere, Category = Notify)
	FGameplayTagContainer IdentityTags;
	
	UPROPERTY(EditAnywhere, Category = Notify)
	FGameplayTag NotifyTag;
};

USTRUCT(Category = "Flicker Void", meta = (DisplayName = "Notify FlowGraph"))
struct FLICKERVOIDCORE_API FFVSTT_NotifyFlowGraph : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FNotifyFlowGraphInstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FNotifyFlowGraphInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(
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
		return NSLOCTEXT("FlickerVoid", "NotifyFlowGraphTask", "Notify FlowGraph");
	}
#endif
	
};
