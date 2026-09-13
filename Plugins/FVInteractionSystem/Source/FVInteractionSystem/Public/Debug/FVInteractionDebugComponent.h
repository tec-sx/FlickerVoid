#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "FVInteractionDebugComponent.generated.h"

class APlayerController;
class UCanvas;
class UFVInteractorComponent;

UCLASS(MinimalAPI, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UFVInteractionDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionDebugComponent();

#if !UE_BUILD_SHIPPING
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RefreshCachedInteractor();
	void GetViewPoint(FVector& OutPawnLocation, FVector& OutViewLocation, FVector& OutForward) const;
	void DrawVisualizer() const;
	void DrawHUD(UCanvas* Canvas, APlayerController* PC);

	TWeakObjectPtr<UFVInteractorComponent> Interactor;
	FDelegateHandle HUDDrawHandle;
#endif
};
