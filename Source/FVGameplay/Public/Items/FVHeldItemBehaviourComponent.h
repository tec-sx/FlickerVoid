#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVHeldItemBehaviourComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnHeldItemUsed, AActor*, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnHeldItemToggled, bool, bOn);

/** Goes on a held item's actor class (e.g. a flashlight Blueprint): using the held item broadcasts On Used, and a toggle flips On and broadcasts On Toggled for the Blueprint to switch its light. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVHeldItemBehaviourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVHeldItemBehaviourComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Held", meta = (ToolTip = "Called by the holder's Held Item component: flips On for a toggle, then broadcasts On Used."))
	void Use(AActor* User);

	UFUNCTION(BlueprintPure, Category = "FV|Item|Held", meta = (ToolTip = "Current toggle state."))
	bool IsOn() const { return bOn; }

	UPROPERTY(BlueprintAssignable, Category = "FV|Item|Held", meta = (ToolTip = "The held item was used, with the character using it."))
	FFVOnHeldItemUsed OnUsed;

	UPROPERTY(BlueprintAssignable, Category = "FV|Item|Held", meta = (ToolTip = "On changed (Toggle State only)."))
	FFVOnHeldItemToggled OnToggled;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", meta = (ToolTip = "Each use flips On, e.g. a flashlight's light."))
	bool bToggleState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", meta = (ToolTip = "Current toggle state; also the starting one."))
	bool bOn = false;
};
