// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "FVAsyncAction_LestenForFactChanges.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsyncFactDelegate, int32, CurrentValue);

UCLASS()
class FLICKERVOIDCORE_API UFVAsyncAction_LestenForFactChanges : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "Messaging",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", Categories="Fact"))
	static UFVAsyncAction_LestenForFactChanges* ListenForFactChanges(UObject* WorldContextObject, FGameplayTag Tag);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;
	
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Value Changed"))
	FAsyncFactDelegate OnFactValueChanged;
	
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "Became Defined"))
	FAsyncFactDelegate OnFactBecameDefined;
private:
	UFUNCTION()
	void HandleFactValueChanged( int32 CurrentValue );

	UFUNCTION()
	void HandleFactBecameDefined( int32 CurrentValue );
	
	TWeakObjectPtr< UWorld > WorldPtr;
	FGameplayTag Tag;
};
