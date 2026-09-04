#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#include "InteractionRequirement.generated.h"

class UInteractorComponent;
class UInteractableComponent;

UENUM(BlueprintType)
enum class EInteractionGate : uint8
{
	Hide,
	Disable,
};

UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class FVINTERACTIONSYSTEM_API UInteractionRequirement : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool IsMet(const FGameplayTag ActionTag, const UInteractorComponent* Interactor, const UInteractableComponent* Interactable) const;
	virtual bool IsMet_Implementation(const FGameplayTag ActionTag, const UInteractorComponent* Interactor, const UInteractableComponent* Interactable) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionGate Gate = EInteractionGate::Disable;
};
