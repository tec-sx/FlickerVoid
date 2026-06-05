// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Interfaces/FVInteractableInterface.h"
#include "FVItemPickupActor.generated.h"

#define UE_API FLICKERVOIDITEMS_API

class UBoxComponent;
class UFVInventoryItemBase;
class UFVInventoryComponent;
class UFVInteractionActionData;

/**
 * World actor representing an item that can be picked up.
 * Implements IFVInteractableInterface for GAS-based interaction system.
 */
UCLASS(MinimalAPI)
class AFVItemPickupActor : public AActor, public IFVInteractableInterface
{
	GENERATED_BODY()

public:
	AFVItemPickupActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	// ============================================================================
	// IFVInteractableInterface Implementation
	// ============================================================================
	
	virtual FGameplayTag GetInteractableTag() const override;
	FGameplayTagContainer GetAvailableActions_Implementation() const;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Item")
	UE_API bool ExecutePickup(UFVInventoryComponent* TargetInventory);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Item")
	UE_API int32 GetQuantity() const { return Quantity; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Item")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Item")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FlickerVoid|Item")
	TObjectPtr<class UWidgetComponent> InfoWidgetComponent;

	UPROPERTY(BlueprintReadWrite, Category = "FlickerVoid|Item")
	TObjectPtr<UFVInventoryItemBase> ItemData;

	UPROPERTY(BlueprintReadWrite, Category = "FlickerVoid|Item")
	int32 Quantity;

	/** Highlight color when focused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlickerVoid|Item|Highlight")
	FLinearColor HighlightColor;

	/** Highlight intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlickerVoid|Item|Highlight")
	float HighlightIntensity;

	UPROPERTY(BlueprintReadOnly, Category = "FlickerVoid|Item")
	bool bIsFocused;

	/** Interaction action data assets - configure in Blueprint or defaults */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FlickerVoid|Item|Interaction")
	TObjectPtr<UFVInteractionActionData> PickupAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FlickerVoid|Item|Interaction")
	TObjectPtr<UFVInteractionActionData> ExamineAction;

	/** Blueprint event when focused */
	UFUNCTION(BlueprintImplementableEvent, Category = "FlickerVoid|Item")
	void BP_OnFocusGained();

	/** Blueprint event when focus lost */
	UFUNCTION(BlueprintImplementableEvent, Category = "FlickerVoid|Item")
	void BP_OnFocusLost();

private:
	FVector OriginalLocation;
	float FloatHeight;
	float TimeAccumulator;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
	
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	/** Cache of interaction tags */
	FGameplayTagContainer CachedInteractionTags;

	void SetupVisualFeedback(bool bEnabled);
};

#undef UE_API
