#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "InteractionDebugComponent.generated.h"

class APlayerController;
class UCanvas;
class UInteractorComponent;

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractionDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionDebugComponent();

#if !UE_BUILD_SHIPPING
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RefreshCachedInteractor();
	void GetViewPoint(FVector& OutLocation, FVector& OutForward) const;
	void DrawVisualizer() const;
	void DrawHUD(UCanvas* Canvas, APlayerController* PC);

	TWeakObjectPtr<UInteractorComponent> Interactor;
	FDelegateHandle HUDDrawHandle;
#endif
};
