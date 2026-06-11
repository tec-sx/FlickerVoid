#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FVHUD.generated.h"

class UFVInteractionPromptWidget;

UCLASS()
class FLICKERVOIDUI_API AFVHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "UI|Interaction")
	UFVInteractionPromptWidget* GetInteractionPromptWidget() const { return InteractionPromptWidget; }

	UFUNCTION(BlueprintPure, Category = "UI|Dialogue")
	UUserWidget* GetDialogueWidget() const { return DialogueWidget; }

protected:
	// Set in the HUD blueprint — assign your UFVInteractionPromptWidgetImpl subclass
	UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<UFVInteractionPromptWidget> InteractionPromptWidgetClass;

	// Set in the HUD blueprint — assign your UFVDialogueWidget subclass
	// (ZOrder 1 — renders above the interaction prompt)
	UPROPERTY(EditDefaultsOnly, Category = "UI|Dialogue")
	TSubclassOf<UUserWidget> DialogueWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UFVInteractionPromptWidget> InteractionPromptWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> DialogueWidget;
};
