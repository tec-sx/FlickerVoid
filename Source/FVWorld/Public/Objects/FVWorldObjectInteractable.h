#pragma once

#include "CoreMinimal.h"
#include "FVWorldObjectBase.h"
#include "Interfaces/FVInteractableInterface.h"
#include "FVWorldObjectInteractable.generated.h"

UCLASS()
class FLICKERVOIDWORLD_API AFVWorldObjectInteractable : 
	public AFVWorldObjectBase, 
	public IFVInteractableInterface
{
	GENERATED_BODY()

public:
	AFVWorldObjectInteractable();

	// Object Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identification", meta = (Categories = "WorldObject"))
	FGameplayTag ObjectTypeTag;

	// Interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (Categories = "Interaction.Action"))
	FGameplayTagContainer AvailableActions;

	// IFVInteractableInterface
	virtual FGameplayTag GetInteractableTag() const override;

	void OnFocusGained_Implementation(AActor* Interactor);
	void OnFocusLost_Implementation(AActor* Interactor);
	FGameplayTagContainer GetAvailableActions_Implementation() const;
};
