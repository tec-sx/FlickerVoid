

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

	UPROPERTY(Config, EditAnywhere, Category = "Input", meta = (ForceInlineRow, Categories = "InputTag.Interaction"))
	TMap<FGameplayTag, FInteractionKeyBinding> InputBindings;

	const FInteractionKeyBinding* FindInputBinding(const FGameplayTag& InputTag) const;
};
