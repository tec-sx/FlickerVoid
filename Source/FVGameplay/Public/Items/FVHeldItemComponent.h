#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FVHeldItemComponent.generated.h"

class UAnimSequenceBase;
class UFVEquipmentComponent;
class UFVItemDefinition;
class USkeletalMeshComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnHeldItemsChanged);

/**
 * Shows what the owner holds in its hand slots (Equipment.Slot.Hand.Right/Left): the item's World actor class, or a
 * mesh prop, at the hand socket. Abilities that need both hands put the held items away. The owner passes the hold
 * poses to its anim instance on On Held Items Changed.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVHeldItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVHeldItemComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Held", meta = (ToolTip = "Hides the held items and clears the hold poses while an action needs both hands. Every Put Away needs its own Take Out."))
	void PutAway();

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Held", meta = (ToolTip = "Shows the held items again after Put Away."))
	void TakeOut();

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Held", meta = (ToolTip = "Uses the item held in that hand: its held actor's Held Item Behaviour component, else Event.Item.Use for a usable item. False if nothing happened."))
	bool UseHeld(bool bRightHand);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Held", meta = (ToolTip = "Upper body pose of the item held in that hand (its Hold Style); none when the hand is empty or the held items are put away."))
	UAnimSequenceBase* GetHoldPose(bool bRightHand) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleEquipmentChanged(FGameplayTag Slot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem);

	UFUNCTION()
	void HandleEquipmentReset();

	void RefreshHand(bool bRightHand);

	/** The World fragment's Actor Class at the hand socket, without collision or physics; none without one. A class with a pickup (a world item) gets the held unit before construction, so it looks like the dropped item. */
	AActor* SpawnHeldActor(UFVItemDefinition* Item, const FGameplayTag& Slot, USkeletalMeshComponent* CharacterMesh, FName Socket) const;

	void SetHand(bool bRightHand, AActor* HeldActor, UStaticMeshComponent* Prop);
	void ClearHand(bool bRightHand);
	void RefreshPresentation();

public:
	UPROPERTY(BlueprintAssignable, Category = "FV|Item|Held", meta = (ToolTip = "What the hands show changed: an item went into or out of a hand, or the held items were put away or taken out. Read Get Hold Pose for the anim instance."))
	FFVOnHeldItemsChanged OnHeldItemsChanged;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFVEquipmentComponent> Equipment;

	UPROPERTY(Transient)
	TObjectPtr<AActor> RightActor;

	UPROPERTY(Transient)
	TObjectPtr<AActor> LeftActor;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> RightProp;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> LeftProp;

	int32 BusyCount = 0;
};
