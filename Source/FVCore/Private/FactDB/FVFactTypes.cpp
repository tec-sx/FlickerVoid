#include "FactDB/FVFactTypes.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactTypes)

bool FFVFactCondition::IsValid() const
{
	return Tag.IsValid();
}

FString FFVFactCondition::ToString() const
{
	return FString::Format( TEXT( "{0} {1} {2}" ), { Tag.ToString()
			, UEnum::GetDisplayValueAsText( Operator ).ToString()
			, FString::FromInt( WantedValue ) } );
}

bool FFVFactCondition::CheckValue(const UFVFactSubsystem& FactSubsystem) const
{
	return FactSubsystem.CheckFactCondition(*this);
}

bool FFVFactConditionGroup::IsValid() const
{
	if (Conditions.IsEmpty())
	{
		return false;
	}

	for (const FFVFactCondition& Condition : Conditions)
	{
		if (!Condition.IsValid())
		{
			return false;
		}
	}

	return true;
}

FString FFVFactConditionGroup::ToString() const
{
	if (Conditions.IsEmpty())
	{
		return TEXT("<no conditions>");
	}

	const TCHAR* Separator = LogicOp == EFVFactLogicOp::AllOf ? TEXT("\nAND ") : TEXT("\nOR ");

	TArray<FString> Parts;
	Parts.Reserve(Conditions.Num());
	for (const FFVFactCondition& Condition : Conditions)
	{
		Parts.Add(Condition.ToString());
	}

	return FString::Join(Parts, Separator);
}
