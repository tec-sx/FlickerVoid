#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "FVScannerComponent.generated.h"

class UFVScannableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanFocusChanged, UFVScannableComponent*, Scannable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanCompleted, UFVScannableComponent*, Scannable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScannerModeChanged, bool, bActive);

/**
 * Player scanner. In scan mode it reveals scannables in range, focuses the one closest to the view
 * direction, and scans it while scanning is held.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVSCANNERSYSTEM_API UFVScannerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVScannerComponent();

	UFUNCTION(BlueprintCallable, Category = "FV|Scanner")
	void SetScanMode(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "FV|Scanner")
	void ToggleScanMode() { SetScanMode(!bScanMode); }

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool IsScanModeActive() const { return bScanMode; }

	/** Begins scanning the focus; call from the scan input's pressed event. */
	UFUNCTION(BlueprintCallable, Category = "FV|Scanner")
	void StartScan();

	UFUNCTION(BlueprintCallable, Category = "FV|Scanner")
	void StopScan();

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	UFVScannableComponent* GetFocus() const { return Focus.Get(); }

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	float GetProgress() const { return Progress; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScannerModeChanged OnScanModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanProgress OnProgress;

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanCompleted OnScanCompleted;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner", meta = (ClampMin = 0, Units = "cm"))
	float Range = 2500.f;

	/** Max angle between view direction and target to be focused. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner", meta = (ClampMin = 0, ClampMax = 90, Units = "deg"))
	float FocusAngle = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Real seconds between reveal/focus updates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner", meta = (ClampMin = 0.02, Units = "s"))
	float UpdateInterval = 0.1f;

private:
	void UpdateTargets();
	void SetFocus(UFVScannableComponent* NewFocus);
	void ClearRevealed();
	void GetViewPoint(FVector& OutLocation, FVector& OutDirection) const;
	bool HasLineOfSight(const FVector& From, const UFVScannableComponent& Scannable) const;
	float GetRealDeltaTime(float DeltaTime) const;

	TWeakObjectPtr<UFVScannableComponent> Focus;
	TArray<TWeakObjectPtr<UFVScannableComponent>> Revealed;
	float Progress = 0.f;
	float UpdateAccumulator = 0.f;
	bool bScanMode = false;
	bool bScanning = false;
};
