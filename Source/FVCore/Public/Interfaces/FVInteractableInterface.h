#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "FVInteractableInterface.generated.h"

class UFVInteractionActionData;

UINTERFACE(MinimalAPI, BlueprintType)
class UFVInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class FLICKERVOIDCORE_API IFVInteractableInterface
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetInteractableTag() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnFocusGained(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void OnFocusLost(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	FGameplayTagContainer GetAvailableActions() const;
};
