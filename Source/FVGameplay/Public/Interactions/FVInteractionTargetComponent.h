#pragma once

#include "CoreMinimal.h"
#include "FVInteractionTargetConfig.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Interactions/FVInteractionTypes.h"

#include "FVInteractionTargetComponent.generated.h"

class UFVInteractionInstigatorComponent;
class UFVInteractionTargetConfig;

UCLASS(Blueprintable, ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVInteractionTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionTargetComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Config
	
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TObjectPtr<UFVInteractionTargetConfig> Config;

	// Runtime State

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInFocus() const { return bIsInFocus; }

	void SetFocused(bool bFocused);
	float GetFocusRadius() const { return Config ? Config->FocusRadius : 0.f; }

	FFVInteractionFocusProfile GetFocusProfile() const
	{
		return Config ? Config->GetFocusProfile() : FFVInteractionFocusProfile::Precise();
	}

	FVector GetAimProbeLocation() const;

	UFUNCTION(BlueprintPure, Category = "Interaction|Actions")
	TArray<UFVInteractionConfig*> GetAvailableInteractions() const;

private:
	void EnsureZoneComponent(AActor* Owner);

	bool bIsInitialized = false;
	bool bIsInFocus = false;
};
