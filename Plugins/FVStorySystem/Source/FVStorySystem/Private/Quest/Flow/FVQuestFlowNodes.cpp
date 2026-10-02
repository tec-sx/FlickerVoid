#include "Quest/Flow/FVQuestFlowNodes.h"

#include "Dialogue/FVDialogueBridge.h"
#include "Facts/FVFactDatabase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVQuestFlowNodes)

namespace FVQuestFlow
{
	FString DescribeQuest(const UFVQuestDefinition* Quest, EFVQuestState State)
	{
		const FString Name = Quest ? Quest->Display.Name.ToString() : TEXT("<none>");
		return FString::Printf(TEXT("%s\n= %s"), *Name, *UEnum::GetDisplayValueAsText(State).ToString());
	}
}

UFVFlowNode_SetQuestState::UFVFlowNode_SetQuestState()
{
#if WITH_EDITOR
	Category = TEXT("Quest");
	NodeDisplayStyle = FlowNodeStyle::Default;
#endif
}

void UFVFlowNode_SetQuestState::ExecuteInput(const FName& PinName)
{
	UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(this);
	if (!Subsystem || !Quest)
	{
		LogError(TEXT("Missing quest or quest subsystem"));
	}
	else if (!ApplyState(*Subsystem))
	{
		LogWarning(TEXT("Quest state transition rejected"));
	}

	TriggerFirstOutput(true);
}

bool UFVFlowNode_SetQuestState::ApplyState(UFVQuestSubsystem& Subsystem) const
{
	switch (State)
	{
	case EFVQuestState::Active:    return Subsystem.StartQuest(Quest);
	case EFVQuestState::Completed: return Subsystem.CompleteQuest(Quest);
	case EFVQuestState::Failed:    return Subsystem.FailQuest(Quest);
	default:                       return false;
	}
}

#if WITH_EDITOR
FString UFVFlowNode_SetQuestState::GetNodeDescription() const
{
	return FVQuestFlow::DescribeQuest(Quest, State);
}

EDataValidationResult UFVFlowNode_SetQuestState::ValidateNode()
{
	if (!Quest)
	{
		ValidationLog.Error<UFlowNode>(TEXT("Quest is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif

UFVFlowNode_CompleteObjective::UFVFlowNode_CompleteObjective()
{
#if WITH_EDITOR
	Category = TEXT("Quest");
	NodeDisplayStyle = FlowNodeStyle::Default;
#endif
}

void UFVFlowNode_CompleteObjective::ExecuteInput(const FName& PinName)
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->SetFact(ObjectiveFact, 1);
	}
	else
	{
		LogError(TEXT("No fact database"));
	}

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FString UFVFlowNode_CompleteObjective::GetNodeDescription() const
{
	return ObjectiveFact.IsValid() ? ObjectiveFact.ToString() : TEXT("None");
}

EDataValidationResult UFVFlowNode_CompleteObjective::ValidateNode()
{
	if (!ObjectiveFact.IsValid())
	{
		ValidationLog.Error<UFlowNode>(TEXT("Objective fact is required"), this);
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif

UFVFlowNode_WaitForQuestState::UFVFlowNode_WaitForQuestState()
{
#if WITH_EDITOR
	Category = TEXT("Quest");
	NodeDisplayStyle = FlowNodeStyle::Latent;
#endif
}

void UFVFlowNode_WaitForQuestState::ExecuteInput(const FName& PinName)
{
	if (TryFinish())
	{
		return;
	}

	UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(this);
	if (!Subsystem || !Quest)
	{
		LogError(TEXT("Missing quest or quest subsystem"));
		TriggerFirstOutput(true);
		return;
	}

	BoundSubsystem = Subsystem;
	Subsystem->OnQuestsChanged.AddUniqueDynamic(this, &UFVFlowNode_WaitForQuestState::HandleQuestsChanged);
}

void UFVFlowNode_WaitForQuestState::HandleQuestsChanged()
{
	TryFinish();
}

bool UFVFlowNode_WaitForQuestState::TryFinish()
{
	const UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(this);
	if (!Subsystem || !Quest || Subsystem->GetState(Quest) != State)
	{
		return false;
	}

	TriggerFirstOutput(true);
	return true;
}

void UFVFlowNode_WaitForQuestState::Cleanup()
{
	if (UFVQuestSubsystem* Subsystem = BoundSubsystem.Get())
	{
		Subsystem->OnQuestsChanged.RemoveDynamic(this, &UFVFlowNode_WaitForQuestState::HandleQuestsChanged);
	}
	BoundSubsystem.Reset();
	Super::Cleanup();
}

#if WITH_EDITOR
FString UFVFlowNode_WaitForQuestState::GetNodeDescription() const
{
	return FVQuestFlow::DescribeQuest(Quest, State);
}
#endif

UFVFlowNodeAddOn_QuestStatePredicate::UFVFlowNodeAddOn_QuestStatePredicate()
{
#if WITH_EDITOR
	Category = TEXT("Quest");
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
#endif
}

bool UFVFlowNodeAddOn_QuestStatePredicate::EvaluatePredicate_Implementation() const
{
	const UFVQuestSubsystem* Subsystem = UFVQuestSubsystem::Get(this);
	return Subsystem && Quest && Subsystem->GetState(Quest) == State;
}

UFVFlowNodeAddOn_ConditionPredicate::UFVFlowNodeAddOn_ConditionPredicate()
{
#if WITH_EDITOR
	Category = TEXT("FlickerVoid");
	NodeDisplayStyle = FlowNodeStyle::AddOn_Predicate;
#endif
}

bool UFVFlowNodeAddOn_ConditionPredicate::EvaluatePredicate_Implementation() const
{
	return Conditions.Evaluate(FVDialogueBridge::MakeContext(GetWorld()));
}
