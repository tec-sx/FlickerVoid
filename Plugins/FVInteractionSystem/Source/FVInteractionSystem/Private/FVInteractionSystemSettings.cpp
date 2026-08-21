#include "FVInteractionSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionSystemSettings)

UFVInteractionSystemSettings::UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CategoryName = "Flicker Void";
	SectionName = "Interaction System";
	FocusProfiles.Add(TEXT("Precise"), { 10.f, 0.85f, 1.f, 0.2f });
	FocusProfiles.Add(TEXT("Loose"), { 10.f, 0.35f, 0.7f, 0.5f });
}

const FInteractionFocusProfile& UFVInteractionSystemSettings::GetFocusProfile(FName ProfileName) const
{
	if (const FInteractionFocusProfile* Profile = FocusProfiles.Find(ProfileName))
	{
		return *Profile;
	}

	return DefaultFocusProfile;
}

const FInteractionKeyBinding* UFVInteractionSystemSettings::FindInputBinding(const FGameplayTag& InputTag) const
{
	return InputBindings.Find(InputTag);
}

TArray<FString> UFVInteractionSystemSettings::GetFocusProfileNames()
{
	TArray<FString> Names;

	if (const UFVInteractionSystemSettings* Settings = GetDefault<UFVInteractionSystemSettings>())
	{
		Names.Reserve(Settings->FocusProfiles.Num());
		for (const TPair<FName, FInteractionFocusProfile>& Pair : Settings->FocusProfiles)
		{
			Names.Add(Pair.Key.ToString());
		}
		Names.Sort();
	}

	return Names;
}


