// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FVInventorySnapshotProvider.generated.h"

class UFVInventoryEntryObject;

UINTERFACE(BlueprintType, MinimalAPI)
class UFVInventorySnapshotProvider : public UInterface
{
	GENERATED_BODY()
};

class FLICKERVOIDUI_API IFVInventorySnapshotProvider
{
	GENERATED_BODY()

public:
	virtual TArray<TObjectPtr<UFVInventoryEntryObject>> BuildInventorySnapshot() const = 0;
};
