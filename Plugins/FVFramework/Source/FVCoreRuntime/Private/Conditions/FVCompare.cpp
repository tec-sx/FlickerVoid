#include "Conditions/FVCompare.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCompare)

bool FVCompare::Score(const float Current, const EFVCompare Compare, const float Value, FFVConditionScore& OutScore)
{
	if (Compare != EFVCompare::Greater && Compare != EFVCompare::GreaterOrEqual)
	{
		return false;
	}

	OutScore = FFVConditionScore::AtLeast(Current, Value);
	return true;
}

FText FVCompare::GetDisplayName(const EFVCompare Compare)
{
	return StaticEnum<EFVCompare>()->GetDisplayNameTextByValue(static_cast<int64>(Compare));
}
