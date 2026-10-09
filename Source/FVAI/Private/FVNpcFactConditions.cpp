#include "FVNpcFactConditions.h"

#include "FVAIConfigData.h"
#include "FVAIFactHelpers.h"
#include "Facts/FVFactDatabase.h"
#include "FVNpcFactOptions.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNpcFactConditions)

#define LOCTEXT_NAMESPACE "FVNpcFact"

FGameplayTag FVNpcFacts::MakeTag(const UFVAIConfigData* Npc, FName Aspect)
{
	if (!Npc || Npc->NpcId.IsNone() || Aspect.IsNone())
	{
		return FGameplayTag();
	}
	return UFVAIFactHelpers::MakeNpcFactTag(Npc->NpcId, Aspect);
}

TArray<FName> UFVNpcFactOptions::GetAspects()
{
	return { FVNpcFactAspects::Met, FVNpcFactAspects::Alive, FVNpcFactAspects::Trust, FVNpcFactAspects::Disposition };
}

bool FFVCondition_NpcFact::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(Context.WorldContext);
	const FGameplayTag Tag = FVNpcFacts::MakeTag(Npc, Aspect);
	return Database && Tag.IsValid() && FVFacts::Compare(*Database, Tag, Compare, Value);
}

FText FFVCondition_NpcFact::GetDescription() const
{
	const FText Name = Npc ? FText::FromName(Npc->NpcId) : LOCTEXT("None", "<none>");
	const FText Op = StaticEnum<EFVFactCompare>()->GetDisplayNameTextByValue(static_cast<int64>(Compare));
	return FText::Format(LOCTEXT("Cond", "{0}.{1} {2} {3}"), Name, FText::FromName(Aspect), Op, Value);
}

void FFVEffect_NpcFact::Apply(const FFVConditionContext& Context) const
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(Context.WorldContext);
	const FGameplayTag Tag = FVNpcFacts::MakeTag(Npc, Aspect);
	if (!Database || !Tag.IsValid())
	{
		return;
	}

	switch (Write)
	{
	case EFVFactWrite::Set:    Database->SetFact(Tag, Value); break;
	case EFVFactWrite::Add:    Database->AddFact(Tag, Value); break;
	case EFVFactWrite::Remove: Database->RemoveFact(Tag); break;
	}
}

FText FFVEffect_NpcFact::GetDescription() const
{
	const FText Name = Npc ? FText::FromName(Npc->NpcId) : LOCTEXT("None", "<none>");
	const FText Op = StaticEnum<EFVFactWrite>()->GetDisplayNameTextByValue(static_cast<int64>(Write));
	return FText::Format(LOCTEXT("Eff", "{1} {0}.{2} {3}"), Name, Op, FText::FromName(Aspect), Value);
}

#undef LOCTEXT_NAMESPACE