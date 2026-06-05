// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVMovementHandlerInfo.generated.h"

class UFVMovementHandlerBase;
class UFVMovementHandlerConfigBase;

UENUM(BlueprintType)
enum class EFVMovementHandlerPriority : uint8
{
	Lowest = 0		UMETA(DisplayName = "Lowest"),
	Low = 25		UMETA(DisplayName = "Low"),
	Normal = 50		UMETA(DisplayName = "Normal"),
	High = 75		UMETA(DisplayName = "High"),
	Highest = 100	UMETA(DisplayName = "Highest"),
	Critical = 255	UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVMovementHandlerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Handler", meta = (MustImplement = "/Script/FlickerVoidCharacter.FVMovementHandlerBase"))
	TSubclassOf<UFVMovementHandlerBase> HandlerClass;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Movement Handler", meta = (EditInline))
	TObjectPtr<UFVMovementHandlerConfigBase> HandlerConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation|Tags")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation|Tags")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation|Tags")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation", meta = (ClampMin = "0", ClampMax = "255"))
	uint8 Priority = static_cast<uint8>(EFVMovementHandlerPriority::Normal);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Documentation", meta = (MultiLine = true))
	FString Description;

	FFVMovementHandlerInfo()
		: HandlerClass(nullptr)
		, HandlerConfig(nullptr)
		, Priority(static_cast<uint8>(EFVMovementHandlerPriority::Normal))
		, bCanBeInterrupted(true)
	{
	}

	bool CanActivate(const FGameplayTagContainer& CurrentTags) const
	{
		if (!RequiredTags.IsEmpty() && !CurrentTags.HasAll(RequiredTags))
		{
			return false;
		}

		if (CurrentTags.HasAny(BlockedByTags))
		{
			return false;
		}

		return true;
	}

	bool HasHigherPriorityThan(const FFVMovementHandlerInfo& Other) const
	{
		return Priority > Other.Priority;
	}
};
