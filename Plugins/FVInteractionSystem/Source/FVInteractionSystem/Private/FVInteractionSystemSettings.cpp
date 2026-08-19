#include "FVInteractionSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionSystemSettings)

UFVInteractionSystemSettings::UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = "Flicker Void";
	SectionName = "Interaction System";
	FocusProfiles = 
	{
		{
			.Name = "Precise",
			.ConeCosine = 0.85f,
			.AngularWeight = 1.f,
			.DistanceWeight = 0.2f
		},
		{
			.Name = "Loose",
			.ConeCosine = 0.35f,
			.AngularWeight = 0.7f,
			.DistanceWeight = 0.5f
		}
	};
}


