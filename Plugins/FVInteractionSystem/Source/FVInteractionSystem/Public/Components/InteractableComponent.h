

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Core/InteractionTypes.h"
#include "GameplayTags.h"
#include "InteractableComponent.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractableComponent : public USphereComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer AvailableInteractions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	FName AimProbeSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	FVector AimProbeOffset = FVector::ZeroVector;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FInteractionFocusProfile GetFocusProfile() const { return FocusProfile; }
	
	FVector GetAimProbeLocation() const;
	void SetFocused(bool bFocused);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API bool IsInFocus() const { return bIsInFocus; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = ( AllowPrivateAcces = true))
	FInteractionFocusProfile FocusProfile;

	bool bIsInitialized = false;
	bool bIsInFocus = false;

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
};

#undef UE_API
