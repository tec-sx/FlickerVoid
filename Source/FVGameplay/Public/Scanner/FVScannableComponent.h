#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Scanner/FVScanDefinition.h"
#include "FVScannableComponent.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanRevealChanged, bool, bRevealed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnScanned, AActor*, Scanner);

/**
 * Makes the owner show up in scan mode and reveal its scan definition when scanned. The owner also
 * needs an interactable component, since the scanner finds its targets through the interaction registry.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVScannableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVScannableComponent();

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	UFVScanDefinition* GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintCallable, Category = "FV|Scanner")
	void SetDefinition(UFVScanDefinition* NewDefinition) { Definition = NewDefinition; }

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool CanBeScannedBy(AActor* Scanner) const;

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool WasScanned() const { return bScanned; }

	/** Entries whose conditions pass for this scanner. */
	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	TArray<FFVScanEntry> GetVisibleEntries(AActor* Scanner) const;

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	FVector GetScanLocation() const;

	/** Applies the definition's effects on the first scan and broadcasts OnScanned. */
	void CompleteScan(AActor* Scanner);

	/** Highlights or clears the owner's primitives with the category stencil. */
	void SetRevealed(bool bInRevealed);

	UFUNCTION(BlueprintPure, Category = "FV|Scanner")
	bool IsRevealed() const { return bRevealed; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanRevealChanged OnRevealChanged;

	UPROPERTY(BlueprintAssignable, Category = "FV|Scanner")
	FFVOnScanned OnScanned;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scanner")
	TObjectPtr<UFVScanDefinition> Definition;

	UPROPERTY(SaveGame)
	bool bScanned = false;

private:
	struct FPrimitiveState
	{
		TWeakObjectPtr<UPrimitiveComponent> Primitive;
		bool bRenderCustomDepth = false;
		int32 Stencil = 0;
	};

	TArray<FPrimitiveState> SavedStates;
	bool bRevealed = false;
};
