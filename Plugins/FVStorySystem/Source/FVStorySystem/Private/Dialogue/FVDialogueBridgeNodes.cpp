#include "Dialogue/FVDialogueBridge.h"
#include "Dialogue/FVDialogueSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueBridge)

#define LOCTEXT_NAMESPACE "FVDialogueBridge"

FFVConditionContext FVDialogueBridge::MakeContext(UObject* WorldContext)
{
FFVConditionContext Context;
Context.WorldContext = WorldContext;
Context.Instigator = UGameplayStatics::GetPlayerPawn(WorldContext, 0);
Context.Target = UFVDialogueSubsystem::FindSpeakerActorForContext(WorldContext);
return Context;
}

bool UFVYapCondition::EvaluateCondition_Implementation() const
{
return Conditions.Evaluate(FVDialogueBridge::MakeContext(GetWorld()));
}

#if WITH_EDITOR
FText UFVYapCondition::GetTitle_Implementation() const
{
return Conditions.IsEmpty() ? Super::GetTitle_Implementation() : Conditions.GetDescription();
}
#endif

namespace FVBranchPins
{
const FName In = TEXT("In");
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
const bool bPassed = Conditions.Evaluate(FVDialogueBridge::MakeContext(GetWorld()));
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
Effects.Apply(FVDialogueBridge::MakeContext(GetWorld()));
TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_ApplyEffects::GetNodeDescription() const
{
return Effects.GetDescription().ToString();
}
#endif

#undef LOCTEXT_NAMESPACE