#include "StateTree/FVStateTreeNodes.h"

#include "StateTreeExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVStateTreeNodes)

namespace FVStateTree
{
template <typename TData>
FFVConditionContext MakeContext(const FStateTreeExecutionContext& Context, const TData& Data)
{
FFVConditionContext Result;
Result.WorldContext = Context.GetOwner();
Result.Instigator = Data.Instigator;
Result.Target = Data.Target;
return Result;
}
}

bool FFVSTCondition_Conditions::TestCondition(FStateTreeExecutionContext& Context) const
{
const FInstanceDataType& Data = Context.GetInstanceData(*this);
return Data.Conditions.Evaluate(FVStateTree::MakeContext(Context, Data));
}

EStateTreeRunStatus FFVSTTask_ApplyEffects::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
const FInstanceDataType& Data = Context.GetInstanceData(*this);
Data.OnEnter.Apply(FVStateTree::MakeContext(Context, Data));
return Data.OnExit.IsEmpty() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FFVSTTask_ApplyEffects::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
const FInstanceDataType& Data = Context.GetInstanceData(*this);
Data.OnExit.Apply(FVStateTree::MakeContext(Context, Data));
}