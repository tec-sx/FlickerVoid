#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVGameplayItemFragments.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVItemLibrary.generated.h"

class AActor;
class UAnimMontage;
class UAnimSequenceBase;
class UFVInteractableDefinition;
class UFVItemDefinition;
class UFVItemEventPayload;
class UFVItemSettings;
class USoundBase;
class UStaticMesh;

/** Item helpers for abilities, world items and script: fragments script can't read, animation lookups, world spawning and item events. */
UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVItemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Item", meta = (ToolTip = "Copies the item's World fragment; false without one."))
	static bool GetWorldFragment(const UFVItemDefinition* Item, FFVItemFragment_World& OutFragment);

	UFUNCTION(BlueprintPure, Category = "FV|Item", meta = (ToolTip = "Copies the item's Animation fragment; false without one."))
	static bool GetAnimationFragment(const UFVItemDefinition* Item, FFVItemFragment_Animation& OutFragment);

	UFUNCTION(BlueprintPure, Category = "FV|Item", meta = (ToolTip = "Copies the item's Throwable fragment; false without one."))
	static bool GetThrowableFragment(const UFVItemDefinition* Item, FFVItemFragment_Throwable& OutFragment);

	UFUNCTION(BlueprintCallable, Category = "FV|Item", meta = (ToolTip = "Loads the World fragment's mesh; none without one."))
	static UStaticMesh* LoadWorldMesh(const UFVItemDefinition* Item);

	UFUNCTION(BlueprintCallable, Category = "FV|Item", meta = (ToolTip = "Loads a sound, e.g. one of the World fragment's sounds; none if unset."))
	static USoundBase* LoadSound(const TSoftObjectPtr<USoundBase>& Sound);

	UFUNCTION(BlueprintCallable, Category = "FV|Item", meta = (ToolTip = "Offers of the item in the world: the World fragment's Interactable, else the Items settings entry of its most specific category, else the default."))
	static UFVInteractableDefinition* FindInteractable(const UFVItemDefinition* Item);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Animation", meta = (ToolTip = "Montage of an animation style: the character's Item Animations fragment, else the Items settings default; each tries the style, then its parents."))
	static UAnimMontage* FindMontage(const AActor* Character, FGameplayTag Style);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Animation", meta = (ToolTip = "The item's Use Montage, else the montage of its use style (Animation.Item.Use without one)."))
	static UAnimMontage* FindUseMontage(const AActor* Character, const UFVItemDefinition* Item);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Animation", meta = (ToolTip = "Upper body pose of the item's hold style (Animation.Item.Hold without one), looked up like Find Montage."))
	static UAnimSequenceBase* FindHoldPose(const AActor* Character, const UFVItemDefinition* Item);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Animation", meta = (ToolTip = "Animation.Item.Pickup.Low, Mid or High by the item's height above the character's feet (Items settings)."))
	static FGameplayTag GetPickupStyle(const AActor* Character, FVector ItemLocation);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Animation", meta = (ToolTip = "Character mesh socket for items held in that hand (Items settings)."))
	static FName GetHandSocket(bool bRightHand);

	UFUNCTION(BlueprintPure, Category = "FV|Item", meta = (ToolTip = "The item is usable and its Use Conditions pass for the user. Doesn't look at any inventory."))
	static bool CanUseItem(const UFVItemDefinition* Item, AActor* User);

	UFUNCTION(BlueprintCallable, Category = "FV|Item", meta = (ToolTip = "Applies the item's Usable effects with the user as instigator and target, if Can Use Item. Doesn't touch any inventory."))
	static bool ApplyUseEffects(const UFVItemDefinition* Item, AActor* User);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Give", meta = (ToolTip = "The giver's inventory holds the item and money, and the receiver's inventory (if any) can take the item. False when there is nothing to give."))
	static bool CanGiveItem(const AActor* Giver, const AActor* Receiver, const UFVItemDefinition* Item, int32 Quantity, int32 Money);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Give", meta = (ToolTip = "Moves the item and money from the giver's inventory to the receiver's (a receiver without one just takes them away). As a gift, the receiver's relationship changes by their gift preferences. No animation, no message."))
	static bool GiveItem(AActor* Giver, AActor* Receiver, UFVItemDefinition* Item, int32 Quantity, int32 Money, bool bAsGift);

	UFUNCTION(BlueprintCallable, Category = "FV|Item", meta = (WorldContext = "WorldContext", ToolTip = "Spawns the World fragment's Actor Class, else the Items settings World Item Class, with the stack on its pickup component (instance id cleared)."))
	static AActor* SpawnWorldItem(UObject* WorldContext, const FFVItemStack& Stack, const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Event", meta = (ToolTip = "Sends an item event to the actor's abilities with an item payload; Instigator = Actor, Target = World Actor, else Other Actor. True if an ability was triggered."))
	static bool SendItemEvent(AActor* Actor, FGameplayTag EventTag, UFVItemDefinition* Item, FGuid InstanceId, AActor* WorldActor, AActor* OtherActor, int32 Quantity = 1);

	/** SendItemEvent with a prepared payload, e.g. a give with Money set. */
	static bool SendItemPayload(AActor* Actor, FGameplayTag EventTag, UFVItemEventPayload* Payload);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Event", meta = (ToolTip = "Item payload of an event sent by Send Item Event; none for other events, e.g. ones from interaction offers."))
	static UFVItemEventPayload* GetItemPayload(const FGameplayEventData& EventData);

	UFUNCTION(BlueprintPure, Category = "FV|Item", meta = (ToolTip = "Project Settings > FlickerVoid > Items."))
	static const UFVItemSettings* GetSettings();
};
