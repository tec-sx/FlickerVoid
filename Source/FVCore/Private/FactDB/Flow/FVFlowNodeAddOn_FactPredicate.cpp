#include "FactDB/Flow/FVFlowNodeAddOn_FactPredicate.h"

#include "FlowSettings.h"
#include "FactDB/FVFactSubsystem.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowNodeAddOn_FactPredicate)

UFVFlowNodeAddOn_FactPredicate::UFVFlowNodeAddOn_FactPredicate()
{
#if WITH_EDITOR
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
#endif
}

bool UFVFlowNodeAddOn_FactPredicate::EvaluatePredicate_Implementation() const
{
	if (UWorld* World = GetWorld())
	{
		UFVFactSubsystem& Subsystem = UFVFactSubsystem::Get(World);
		return Subsystem.CheckFactCondition(Condition);
	}

	LogError("No valid world");
	return false;
}

#if WITH_EDITOR
FText UFVFlowNodeAddOn_FactPredicate::GetNodeTitle() const
{
	if (Condition.IsValid() && GetDefault<UFlowSettings>()->bUseAdaptiveNodeTitles)
	{
		return FText::FromString(Condition.ToString());
	}

	return Super::GetNodeTitle();
}
#endif
