#include "FVNavigationLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationLibrary)

#define LOCTEXT_NAMESPACE "FVNavigationLibrary"

FText UFVNavigationLibrary::FormatDistance(float Distance)
{
	const float Metres = FMath::Max(Distance, 0.f) / 100.f;
	if (Metres < 1000.f)
	{
		return FText::Format(LOCTEXT("Metres", "{0} m"), FText::AsNumber(FMath::RoundToInt(Metres)));
	}

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 1;
	Options.MaximumFractionalDigits = 1;
	return FText::Format(LOCTEXT("Kilometres", "{0} km"), FText::AsNumber(Metres / 1000.f, &Options));
}

#undef LOCTEXT_NAMESPACE
