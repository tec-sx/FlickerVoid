// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Movement/FVMovementHandlerInfo.h"

#include "FVMovementHandlerData.generated.h"

/**
 * Data Asset for configuring movement handlers
 * Provides a data-driven approach to movement system configuration
 */
UCLASS(BlueprintType, meta = (DisplayName = "Movement Handler Config"))
class FLICKERVOIDCHARACTER_API UFVMovementHandlerData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVMovementHandlerData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Handlers", meta = (TitleProperty = "Description"))
	TArray<FFVMovementHandlerInfo> MovementHandlers;

	//~=============================================================================
	// Debug Settings
	//~=============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugVisualization = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableVerboseLogging = false;

	//~=============================================================================
	// Helper Functions
	//~=============================================================================

	UFUNCTION(BlueprintCallable, Category = "Movement Handler")
	TArray<FFVMovementHandlerInfo> FindActivatableHandlers(const FGameplayTagContainer& CurrentTags) const;

	UFUNCTION(BlueprintCallable, Category = "Movement Handler")
	bool FindHighestPriorityHandler(const FGameplayTagContainer& CurrentTags, FFVMovementHandlerInfo& OutConfig) const;

	UFUNCTION(BlueprintCallable, Category = "Movement Handler")
	bool ValidateConfiguration(FString& OutErrorMessage) const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& ValidationContext) const override;
#endif
};
