#include "FVCoreFlowNodes.h"

#include "Facts/FVFactDatabase.h"
#include "FlowTags.h"
#include "FVFlowContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCoreFlowNodes)

namespace FVBranchPins
{
	const FName True = TEXT("True");
	const FName False = TEXT("False");
}

UFVFlowNode_Branch::UFVFlowNode_Branch()
{
#if WITH_EDITOR
	Category = TEXT("FlickerVoid");
	NodeDisplayStyle = FlowNodeStyle::Logic;
#endif
	OutputPins.Empty();
	OutputPins.Add(FFlowPin(FVBranchPins::True));
	OutputPins.Add(FFlowPin(FVBranchPins::False));
}

void UFVFlowNode_Branch::ExecuteInput(const FName& PinName)
{
	const bool bPassed = Conditions.Evaluate(FVFlow::MakeContext(*this));
	TriggerOutput(bPassed ? FVBranchPins::True : FVBranchPins::False, true);
}

#if WITH_EDITOR
FString UFVFlowNode_Branch::GetNodeDescription() const
{
	return Conditions.GetDescription().ToString();
}
#endif

UFVFlowNode_ApplyEffects::UFVFlowNode_ApplyEffects()
{
#if WITH_EDITOR
	Category = TEXT("FlickerVoid");
#endif
}

void UFVFlowNode_ApplyEffects::ExecuteInput(const FName& PinName)
{
	Effects.Apply(FVFlow::MakeContext(*this));
	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_ApplyEffects::GetNodeDescription() const
{
	return Effects.GetDescription().ToString();
}
#endif

UFVFlowNode_WaitForConditions::UFVFlowNode_WaitForConditions()
{
#if WITH_EDITOR
	Category = TEXT("FlickerVoid");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFVFlowNode_WaitForConditions::ExecuteInput(const FName& PinName)
{
	if (TryFinish())
	{
		return;
	}

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		FactHandle = Facts->OnFactChangedNative().AddUObject(this, &UFVFlowNode_WaitForConditions::HandleFactChanged);
	}
	else
	{
		LogError(TEXT("No fact database; conditions will never be re-evaluated"));
	}
}

void UFVFlowNode_WaitForConditions::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	TryFinish();
}

bool UFVFlowNode_WaitForConditions::TryFinish()
{
	if (!Conditions.Evaluate(FVFlow::MakeContext(*this)))
	{
		return false;
	}

	TriggerFirstOutput(true);
	return true;
}

void UFVFlowNode_WaitForConditions::Cleanup()
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactHandle);
	}
	FactHandle.Reset();

	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_WaitForConditions::GetNodeDescription() const
{
	return Conditions.GetDescription().ToString();
}
#endif

UFVFlowNodeAddOn_ConditionPredicate::UFVFlowNodeAddOn_ConditionPredicate()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
	Category = TEXT("FlickerVoid");
#endif
}

bool UFVFlowNodeAddOn_ConditionPredicate::EvaluatePredicate_Implementation() const
{
	return Conditions.Evaluate(FVFlow::MakeContext(*this));
}
