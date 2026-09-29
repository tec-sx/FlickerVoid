// Copyright Epic Games, Inc. All Rights Reserved.

#include "Movement/FVMovementHandlerData.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Misc/DataValidation.h"
#include "Movement/FVMovementHandlerBase.h"
#include "Movement/FVMovementHandlerConfigBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMovementHandlerData)

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
	const FFVMovementHandlerInfo* BestConfig = nullptr;

	for (const FFVMovementHandlerInfo& Config : MovementHandlers)
	{
		if (!Config.CanActivate(CurrentTags))
		{
			continue;
		}

		if (!BestConfig || Config.HasHigherPriorityThan(*BestConfig))
		{
			BestConfig = &Config;
		}
	}

	if (!BestConfig)
	{
		return false;
	}

	OutConfig = *BestConfig;
	return true;
}

bool UFVMovementHandlerData::ValidateConfiguration(FString& OutErrorMessage) const
{
	TArray<FString> Errors;
	TSet<UClass*> SeenHandlerClasses;

	for (int32 i = 0; i < MovementHandlers.Num(); ++i)
	{
		const FFVMovementHandlerInfo& Config = MovementHandlers[i];

		if (!Config.HandlerClass)
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d has no handler class assigned"), i));
			continue;
		}

		bool bAlreadySeen = false;
		SeenHandlerClasses.Add(Config.HandlerClass.Get(), &bAlreadySeen);
		if (bAlreadySeen)
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d duplicates handler class %s"), i, *Config.HandlerClass->GetName()));
		}

		if (Config.RequiredTags.HasAny(Config.BlockedByTags))
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d has tags in both RequiredTags and BlockedByTags"), i));
		}

		if (Config.GrantedTags.HasAny(Config.BlockedByTags))
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d grants a tag that blocks itself"), i));
		}

		for (int32 j = i + 1; j < MovementHandlers.Num(); ++j)
		{
			const FFVMovementHandlerInfo& Other = MovementHandlers[j];
			if (Config.GrantedTags.HasAny(Other.GrantedTags))
			{
				Errors.Add(FString::Printf(TEXT("Handlers at indices %d and %d grant overlapping tags"), i, j));
			}
		}

		if (const UFVMovementHandlerBase* HandlerCDO = Config.HandlerClass->GetDefaultObject<UFVMovementHandlerBase>())
		{
			const TSubclassOf<UFVMovementHandlerConfigBase> ExpectedConfigClass = HandlerCDO->GetExpectedConfigClass();
			if (ExpectedConfigClass)
			{
				if (!Config.HandlerConfig)
				{
					Errors.Add(FString::Printf(TEXT("Handler at index %d requires a config of class %s but none is assigned"), i, *ExpectedConfigClass->GetName()));
				}
				else if (!Config.HandlerConfig->IsA(ExpectedConfigClass))
				{
					Errors.Add(FString::Printf(TEXT("Handler at index %d has config of class %s but expects %s"), i, *Config.HandlerConfig->GetClass()->GetName(), *ExpectedConfigClass->GetName()));
				}
			}
		}

		if (Config.Description.IsEmpty())
		{
			Errors.Add(FString::Printf(TEXT("Handler at index %d has no description"), i));
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
