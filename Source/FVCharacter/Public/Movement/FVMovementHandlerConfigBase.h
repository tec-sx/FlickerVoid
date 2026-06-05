// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FVMovementHandlerConfigBase.generated.h"

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class FLICKERVOIDCHARACTER_API UFVMovementHandlerConfigBase : public UObject
{
	GENERATED_BODY()

public:
	UFVMovementHandlerConfigBase() {}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
#endif
};
