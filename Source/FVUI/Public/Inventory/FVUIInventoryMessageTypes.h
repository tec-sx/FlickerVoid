// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVUIInventoryMessageTypes.generated.h"

USTRUCT(BlueprintType)
struct FFVUIInventoryChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid InstanceID;
	UPROPERTY(BlueprintReadOnly) int32 NewQuantity = 0;
};
