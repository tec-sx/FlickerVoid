

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/InteractionTypes.h"
#include "FVInteractionSystemSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FV Interaction System"))
class FVINTERACTIONSYSTEM_API UFVInteractionSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Config, EditAnywhere, Category = "Focus", meta = (DisplayName = "Focus Profiles", ForceInlineRow))
	TMap<FName, FInteractionFocusProfile> FocusProfiles;

	UPROPERTY(Config, EditAnywhere, Category = "Focus")
	FInteractionFocusProfile DefaultFocusProfile;

	UPROPERTY(Config, EditAnywhere, Category = "Input", meta = (ForceInlineRow, Categories = "InputTag.Interaction"))
	TMap<FGameplayTag, FInteractionKeyBinding> InputBindings;

	/** Resolves a profile by name, falling back to DefaultFocusProfile. */
	const FInteractionFocusProfile& GetFocusProfile(FName ProfileName) const;

	const FInteractionKeyBinding* FindInputBinding(const FGameplayTag& InputTag) const;

	UFUNCTION()
	static TArray<FString> GetFocusProfileNames();
};
