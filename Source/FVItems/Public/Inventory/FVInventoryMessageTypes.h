// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVInventoryMessageTypes.generated.h"

USTRUCT(BlueprintType)
struct FFVInventoryChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid InstanceID;
	UPROPERTY(BlueprintReadOnly) int32 NewQuantity = 0;
};
