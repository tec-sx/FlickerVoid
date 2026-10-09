#include "Knowledge/FVKnowledge.h"

#include "Facts/FVFactDatabase.h"
#include "Knowledge/FVKnowledgeDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVKnowledge)

#define LOCTEXT_NAMESPACE "FVKnowledge"

bool UFVKnowledgeLibrary::Knows(const UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge)
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	return Database && Knowledge && Database->GetFact(Knowledge->Id) > 0;
}

bool UFVKnowledgeLibrary::CanLearn(const UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge)
{
	if (!Knowledge || Knows(WorldContext, Knowledge))
	{
		return false;
	}

	for (const UFVKnowledgeDefinition* Prerequisite : Knowledge->Prerequisites)
	{
		if (!Knows(WorldContext, Prerequisite))
		{
			return false;
		}
	}
	return true;
}

bool UFVKnowledgeLibrary::Learn(UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge)
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database || !CanLearn(WorldContext, Knowledge))
	{
		return false;
	}

	Database->SetFact(Knowledge->Id, 1);

	FFVConditionContext Context;
	Context.WorldContext = WorldContext;
	Knowledge->OnLearned.Apply(Context);
	return true;
}

void UFVKnowledgeLibrary::Forget(UObject* WorldContext, const UFVKnowledgeDefinition* Knowledge)
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database || !Knowledge)
	{
		return;
	}
	Database->RemoveFact(Knowledge->Id);
}

bool FFVCondition_Knows::EvaluateImpl(const FFVConditionContext& Context) const
{
	return UFVKnowledgeLibrary::Knows(Context.WorldContext, Knowledge);
}

FText FFVCondition_Knows::GetDescription() const
{
	const FText Name = Knowledge ? Knowledge->Display.Name : LOCTEXT("None", "<none>");
	return FText::Format(LOCTEXT("Knows", "Knows {0}"), Name);
}

void FFVEffect_Learn::Apply(const FFVConditionContext& Context) const
{
	UFVKnowledgeLibrary::Learn(Context.WorldContext, Knowledge);
}

FText FFVEffect_Learn::GetDescription() const
{
	const FText Name = Knowledge ? Knowledge->Display.Name : LOCTEXT("None", "<none>");
	return FText::Format(LOCTEXT("Learn", "Learn {0}"), Name);
}

#undef LOCTEXT_NAMESPACE
