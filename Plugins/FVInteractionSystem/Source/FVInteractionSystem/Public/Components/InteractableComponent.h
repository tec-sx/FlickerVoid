

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Core/InteractionTypes.h"
#include "GameplayTags.h"
#include "InteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractorComponent;

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractableComponent : public USphereComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FInteractionFocusProfile GetFocusProfile() const { return FocusProfile; }
	FVector GetAimProbeLocation() const;
	void SetFocused(bool bFocused);
	
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Identity", meta = (Categories = "Interactable"))
	FGameplayTag Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Actions", meta = (Categories = "Interaction.Action"))
	FGameplayTag PrimaryAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Actions",meta = (Categories = "Interaction.Action"))
	FGameplayTag SecondaryAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interactable|Actions",meta = (Categories = "Interaction.Action"))
	FGameplayTag TernaryAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FName AimProbeSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection")
	FVector AimProbeOffset = FVector::ZeroVector;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Detection", meta = ( AllowPrivateAcces = true))
	FInteractionFocusProfile FocusProfile;
	
private:
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
	bool bIsInitialized = false;
	bool bIsInFocus = false;
};

#undef UE_API
