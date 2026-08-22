

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "GameplayTags.h"
#include "InteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractorComponent;
class UPrimitiveComponent;
class UShapeComponent;

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UE_API FVector GetFocusPoint() const;
	void SetFocused(bool bFocused);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	const FInteractionOffer* FindOffer(const FGameplayTag& InputTag) const { return Offers.Find(InputTag); }
	const TMap<FGameplayTag, FInteractionOffer>& GetOffers() const { return Offers; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection", meta = (ClampMin = "0"))
	float DetectionRadius = 100.f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Actions", meta = (ForceInlineRow, Categories = "InputTag.Interaction"))
	TMap<FGameplayTag, FInteractionOffer> Offers;

	bool bIsInitialized = false;
	bool bIsInFocus = false;

private:
	TWeakObjectPtr<UPrimitiveComponent> FocusPrimitive;
};

#undef UE_API
