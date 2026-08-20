

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "GameplayTags.h"
#include "InteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractorComponent;
class UShapeComponent;

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FInteractionFocusProfile GetFocusProfile() const { return FocusProfile; }
	FVector GetAimProbeLocation() const;
	void SetFocused(bool bFocused);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	const FInteractionOffer* FindOffer(const FGameplayTag& InputTag) const { return Offers.Find(InputTag); }
	const TMap<FGameplayTag, FInteractionOffer>& GetOffers() const { return Offers; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FName AimProbeSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FVector AimProbeOffset = FVector::ZeroVector;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection", meta = ( AllowPrivateAcces = true))
	FInteractionFocusProfile FocusProfile;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Actions", meta = (ForceInlineRow, Categories = "Input.Action"))
	TMap<FGameplayTag, FInteractionOffer> Offers;

	UShapeComponent* InteractionZone;
	bool bIsInitialized = false;
	bool bIsInFocus = false;
};

#undef UE_API
