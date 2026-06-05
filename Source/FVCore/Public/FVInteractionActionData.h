// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FVInteractionActionData.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UTexture2D;
class UInputAction;

/**
 * Defines the type of input required for an interaction.
 */
UENUM(BlueprintType)
enum class EFVInteractionInputType : uint8
{
	Press UMETA(DisplayName = "Press"),
	Hold UMETA(DisplayName = "Hold"),
	Release UMETA(DisplayName = "Release")
};

/**
 * Data asset that defines a single interaction action.
 * Can be configured and reused across different interactables.
 */
UCLASS(MinimalAPI, BlueprintType)
class UFVInteractionActionData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	int32 DisplayPriority;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	EFVInteractionInputType InputType = EFVInteractionInputType::Press;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (EditCondition = "InputType == EFVInteractionInputType::Hold", UIMin = "0.1", UIMax = "5.0"))
	float HoldDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FGameplayTagContainer RequiredActorTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FGameplayTagContainer BlockedByTags;
};

#undef UE_API
