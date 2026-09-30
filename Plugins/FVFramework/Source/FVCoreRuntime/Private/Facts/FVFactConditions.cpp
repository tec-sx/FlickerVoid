#include "Facts/FVFactConditions.h"

#include "Facts/FVFactDatabase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactConditions)

#define LOCTEXT_NAMESPACE "FVFactConditions"

bool FVFacts::Compare(const UFVFactDatabase& Database, FGameplayTag Fact, EFVFactCompare Compare, int32 Value)
{
	const bool bDefined = Database.IsFactDefined(Fact);
	const int32 Current = Database.GetFact(Fact);

	switch (Compare)
	{
	case EFVFactCompare::Equal:          return Current == Value;
	case EFVFactCompare::NotEqual:       return Current != Value;
	case EFVFactCompare::Greater:        return Current > Value;
	case EFVFactCompare::GreaterOrEqual: return Current >= Value;
	case EFVFactCompare::Less:           return Current < Value;
	case EFVFactCompare::LessOrEqual:    return Current <= Value;
	case EFVFactCompare::Defined:        return bDefined;
	case EFVFactCompare::Undefined:      return !bDefined;
	default:                             return false;
	}
}

bool FFVCondition_Fact::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(Context.WorldContext);
	return Database && FVFacts::Compare(*Database, Fact, Compare, Value);
}

FText FFVCondition_Fact::GetDescription() const
{
	const FText Op = StaticEnum<EFVFactCompare>()->GetDisplayNameTextByValue(static_cast<int64>(Compare));
	if (Compare == EFVFactCompare::Defined || Compare == EFVFactCompare::Undefined)
	{
		return FText::Format(LOCTEXT("DescDefined", "{0} {1}"), FText::FromName(Fact.GetTagName()), Op);
	}
	return FText::Format(LOCTEXT("Desc", "{0} {1} {2}"), FText::FromName(Fact.GetTagName()), Op, Value);
}

void FFVEffect_Fact::Apply(const FFVConditionContext& Context) const
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(Context.WorldContext);
	if (!Database)
	{
		return;
	}

	switch (Write)
	{
	case EFVFactWrite::Set:    Database->SetFact(Fact, Value); break;
	case EFVFactWrite::Add:    Database->AddFact(Fact, Value); break;
	case EFVFactWrite::Remove: Database->RemoveFact(Fact); break;
	}
}

FText FFVEffect_Fact::GetDescription() const
{
	const FText Name = FText::FromName(Fact.GetTagName());
	switch (Write)
	{
	case EFVFactWrite::Set: return FText::Format(LOCTEXT("Set", "{0} = {1}"), Name, Value);
	case EFVFactWrite::Add: return FText::Format(LOCTEXT("Add", "{0} += {1}"), Name, Value);
	default:                return FText::Format(LOCTEXT("Remove", "Remove {0}"), Name);
	}
}

#undef LOCTEXT_NAMESPACE
