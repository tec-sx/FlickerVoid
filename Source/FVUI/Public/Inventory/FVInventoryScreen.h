// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "FVInventoryScreen.generated.h"

class UListView;
struct FFVUIInventoryChangedMessage;

UCLASS()
class FLICKERVOIDUI_API UFVInventoryScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UListView> ItemList;

private:
	void HandleInventoryChanged(FGameplayTag Channel, const FFVUIInventoryChangedMessage& Message);
	void RefreshFromSnapshotProvider();

	FGameplayMessageListenerHandle ListenerHandle;
};
