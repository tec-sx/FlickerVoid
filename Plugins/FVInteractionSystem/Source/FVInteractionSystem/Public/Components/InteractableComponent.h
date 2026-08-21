

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
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UE_API const FInteractionFocusProfile& GetFocusProfile() const;
	FVector GetAimProbeLocation() const;
	void SetFocused(bool bFocused);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	const FInteractionOffer* FindOffer(const FGameplayTag& InputTag) const { return Offers.Find(InputTag); }
	const TMap<FGameplayTag, FInteractionOffer>& GetOffers() const { return Offers; }
	FName GetFocusProfileName() const { return FocusProfileName; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FName AimProbeSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FVector AimProbeOffset = FVector::ZeroVector;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Detection", meta = (GetOptions = "FVInteractionSystem.FVInteractionSystemSettings.GetFocusProfileNames"))
	FName FocusProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable|Actions", meta = (ForceInlineRow, Categories = "InputTag.Interaction"))
	TMap<FGameplayTag, FInteractionOffer> Offers;

	bool bIsInitialized = false;
	bool bIsInFocus = false;

private:
	void ResolveFocusProfile();

	const FInteractionFocusProfile* CachedFocusProfile = nullptr;
};

#undef UE_API
