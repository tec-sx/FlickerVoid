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
