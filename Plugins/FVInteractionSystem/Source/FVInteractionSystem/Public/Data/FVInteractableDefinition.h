#pragma once

#include "CoreMinimal.h"
#include "Core/FVInteractionTypes.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVInteractableDefinition.generated.h"

UCLASS(BlueprintType, Const)
class FVINTERACTIONSYSTEM_API UFVInteractableDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVInteractableDefinition();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable", meta = (Categories = "Interactable"))
	FGameplayTag InteractableType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable")
	bool bShowOffers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable|UI")
	EFVFocusIndicatorAnchor FocusIndicatorAnchor = EFVFocusIndicatorAnchor::Center;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable|UI", meta = (Tooltip = "Offset from the anchor in the interactable's local space."))
	FVector FocusIndicatorOffset = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable", meta = (ForceInlineRow))
	TArray<FFVInteractionOffer> Offers;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable", meta = (ClampMin = "0", Units = "s"))
	float CooldownPeriod = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable|Detection")
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable|Detection")
	FGameplayTagContainer CompatibleInteractorTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable|Detection", meta = (ClampMin = "-1"))
	int32 DetectionWeight;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
