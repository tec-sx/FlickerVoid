#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FVHUD.generated.h"

class UFVUILayout;

/**
 * Framework HUD. Creates the FV UI layout (Project Settings > FlickerVoid > UI > LayoutClass)
 * for the owning local player on BeginPlay and releases it on EndPlay.
 * Works with any game mode: just set HUDClass to this (or a Blueprint subclass).
 */
UCLASS(Blueprintable)
class FVCOREUI_API AFVHUD : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|UI")
	UFVUILayout* GetLayout() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called once the layout exists and has been added to the screen. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FV|UI")
	void OnLayoutReady(UFVUILayout* Layout);
};
