#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVInteractionDebugComponent.generated.h"

class UFVInteractionInstigatorComponent;
class UFVInteractionOfferComponent;

// Non-shipping debug utility for the interaction system.
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLICKERVOID_API UFVInteractionDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionDebugComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#if !UE_BUILD_SHIPPING
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFVInteractionInstigatorComponent* GetInstigator() const { return Instigator.Get(); }
	UFVInteractionOfferComponent* GetOfferComponent() const { return OfferComponent.Get(); }

private:
	void RefreshCachedComponents();
	void DrawVisualizer() const;
	void DrawHUD(class UCanvas* Canvas, class APlayerController* PC);

	TWeakObjectPtr<UFVInteractionInstigatorComponent> Instigator;
	TWeakObjectPtr<UFVInteractionOfferComponent> OfferComponent;

	FDelegateHandle HUDDrawHandle;
#endif
};
