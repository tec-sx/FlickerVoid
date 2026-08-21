#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"

#include "InteractionRequirement.generated.h"

class UInteractableComponent;

UENUM(BlueprintType)
enum class EInteractionGate : uint8
{
	Hide,
	Disable,
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionResolveContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const AActor> Interactor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UInteractableComponent> Interactable;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ActionTag;
};

UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories)
class FVINTERACTIONSYSTEM_API UInteractionRequirement : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool IsMet(const FInteractionResolveContext& Context) const;
	virtual bool IsMet_Implementation(const FInteractionResolveContext& Context) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	EInteractionGate Gate = EInteractionGate::Disable;
};
