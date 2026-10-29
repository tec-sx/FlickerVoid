#include "Facts/FVFactConditions.h"

#include "Facts/FVFactDatabase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactConditions)

#define LOCTEXT_NAMESPACE "FVFactConditions"

bool FVFacts::Compare(const UFVFactDatabase& Database, FGameplayTag Fact, EFVFactCompare Compare, int32 Value)
{
	switch (Compare)
	{
	case EFVFactCompare::Defined:   return Database.IsFactDefined(Fact);
	case EFVFactCompare::Undefined: return !Database.IsFactDefined(Fact);
	default:                        return CompareValue(Database.GetFact(Fact), Compare, Value);
	}
}

bool FVFacts::ToCompare(const EFVFactCompare Compare, EFVCompare& OutCompare)
{
	switch (Compare)
	{
	case EFVFactCompare::Equal:          OutCompare = EFVCompare::Equal; return true;
	case EFVFactCompare::NotEqual:       OutCompare = EFVCompare::NotEqual; return true;
	case EFVFactCompare::Greater:        OutCompare = EFVCompare::Greater; return true;
	case EFVFactCompare::GreaterOrEqual: OutCompare = EFVCompare::GreaterOrEqual; return true;
	case EFVFactCompare::Less:           OutCompare = EFVCompare::Less; return true;
	case EFVFactCompare::LessOrEqual:    OutCompare = EFVCompare::LessOrEqual; return true;
	default:                             return false;
	}
}

bool FVFacts::CompareValue(const int32 Current, const EFVFactCompare Compare, const int32 Value)
{
	EFVCompare ValueCompare;
	return ToCompare(Compare, ValueCompare) && FVCompare::Compare(Current, ValueCompare, Value);
}

bool FVFacts::CompareScore(const int32 Current, const EFVFactCompare Compare, const int32 Value, FFVConditionScore& OutScore)
{
	EFVCompare ValueCompare;
	return ToCompare(Compare, ValueCompare) && FVCompare::Score(Current, ValueCompare, Value, OutScore);
}

bool FFVCondition_Fact::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(Context.WorldContext);
	return Database && FVFacts::CompareScore(Database->GetFact(Fact), Compare, Value, OutScore);
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
