// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVMovementHandlerData.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Misc/DataValidation.h"
#include "Movement/FVMovementHandlerBase.h"

UFVMovementHandlerData::UFVMovementHandlerData()
{
	bEnableDebugVisualization = false;
	bEnableVerboseLogging = false;
}

TArray<FFVMovementHandlerInfo> UFVMovementHandlerData::FindActivatableHandlers(const FGameplayTagContainer& CurrentTags) const
{
	TArray<FFVMovementHandlerInfo> ActivatableHandlers;

	for (const FFVMovementHandlerInfo& Config : MovementHandlers)
	{
		if (Config.CanActivate(CurrentTags))
		{
			ActivatableHandlers.Add(Config);
		}
	}

	return ActivatableHandlers;
}

bool UFVMovementHandlerData::FindHighestPriorityHandler(const FGameplayTagContainer& CurrentTags, FFVMovementHandlerInfo& OutConfig) const
{
	bool bFoundHandler = false;
	uint8 HighestPriority = 0;

	for (const FFVMovementHandlerInfo& Config : MovementHandlers)
	{
		if (Config.CanActivate(CurrentTags) && Config.Priority >= HighestPriority)
		{
			HighestPriority = Config.Priority;
			OutConfig = Config;
			bFoundHandler = true;
		}
	}

	return bFoundHandler;
}

bool UFVMovementHandlerData::ValidateConfiguration(FString& OutErrorMessage) const
{
	TArray<FString> Errors;

	for (int32 i = 0; i < MovementHandlers.Num(); ++i)
	{
		const FFVMovementHandlerInfo& Config = MovementHandlers[i];
		
		if (!Config.HandlerClass)
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d has no handler class assigned"), i));
			continue;
		}
	}

	if (MovementHandlers.Num() == 0)
	{
		Errors.Add(TEXT("No movement handlers defined in data asset"));
	}

	if (Errors.Num() > 0)
	{
		OutErrorMessage = FString::Join(Errors, TEXT("\n"));
		return false;
	}

	return true;
}

#if WITH_EDITOR
void UFVMovementHandlerData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate configuration when properties change
	FString ErrorMessage;
	if (!ValidateConfiguration(ErrorMessage))
	{
		FV_LOG_WARNING(LogFVMovement, "Movement Handler Data Asset validation warnings:\n%s", *ErrorMessage);
	}
}

EDataValidationResult UFVMovementHandlerData::IsDataValid(FDataValidationContext& ValidationContext) const
{
	EDataValidationResult Result = Super::IsDataValid(ValidationContext);

	FString ErrorMessage;
	if (!ValidateConfiguration(ErrorMessage))
	{
		ValidationContext.AddError(FText::FromString(ErrorMessage));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
