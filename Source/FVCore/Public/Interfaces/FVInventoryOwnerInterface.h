#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"

#include "FVInventoryOwnerInterface.generated.h"

class AActor;

UINTERFACE(MinimalAPI, BlueprintType)
class UFVInventoryOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

class IFVInventoryOwnerInterface
{
	GENERATED_BODY()

public:
	virtual AActor* GetOwnerActor() = 0;
	virtual AActor* GetOwnerActor() const = 0;

	// Trading support - allows inventory to reference the dealer for price modifiers, etc.
	virtual bool GetTransactionSuccessful() const = 0;
	virtual void SetTransactionSuccessful(bool bSuccessful) = 0;
	virtual AActor* GetDealerActor() = 0;
	virtual AActor* GetDealerActor() const = 0;

	//--------------------------------------------------------------
	// Inventory Actions
	//--------------------------------------------------------------
	
};