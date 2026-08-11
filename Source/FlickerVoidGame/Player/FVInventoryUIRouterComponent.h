// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/FVInventorySnapshotProvider.h"
#include "FVInventoryUIRouterComponent.generated.h"

class UFVInventoryComponent;
class UFVInventoryEntryObject;
struct FFVInventoryChangedMessage;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLICKERVOID_API UFVInventoryUIRouterComponent : public UActorComponent, public IFVInventorySnapshotProvider
{
	GENERATED_BODY()

public:
	UFVInventoryUIRouterComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual TArray<TObjectPtr<UFVInventoryEntryObject>> BuildInventorySnapshot() const override;

private:
	void HandleInventoryChanged(FGameplayTag Channel, const FFVInventoryChangedMessage& Message);

	UFVInventoryComponent* GetInventoryComponent() const;

	FGameplayMessageListenerHandle ListenerHandle;
};
