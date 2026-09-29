#include "Interaction/FVInputGlyphSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInputGlyphSubsystem)

bool UFVInputGlyphSubsystem::ResolveGlyph(FGameplayTag InputTag, FInteractionKeyBinding& OutBinding) const
{
	const FFVInputGlyphEntry* Entry = Glyphs.FindByPredicate([this, InputTag](const FFVInputGlyphEntry& Candidate)
	{
		return Candidate.Device == CurrentDevice && Candidate.InputTag.MatchesTagExact(InputTag);
	});

	if (!Entry)
	{
		return false;
	}

	OutBinding = Entry->Binding;
	return true;
}

void UFVInputGlyphSubsystem::SetInputDevice(EFVInputDevice NewDevice)
{
	if (CurrentDevice == NewDevice)
	{
		return;
	}

	CurrentDevice = NewDevice;
	OnInputDeviceChanged.Broadcast(CurrentDevice);
}
