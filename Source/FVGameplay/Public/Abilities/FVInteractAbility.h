#pragma once

#include "Abilities/FVGameplayAbility.h"
#include "Interactions/FVInteractionTypes.h"
#include "UI/FVInteractionInfo.h"
#include "FVInteractAbility.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class UFVInteractionOfferComponent;
class UFVInteractionTargetComponent;

UCLASS(MinimalAPI, Abstract)
class UFVInteractAbility : public UFVGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UFVInteractionOfferComponent* GetOfferComponent() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UFVInteractionTargetComponent* GetEngagedTarget() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API AActor* GetEngagedActor() const;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UE_API EFVInteractionResult TryBeginInteraction(EFVInteractionSlot InSlot);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	EFVInteractionSlot Slot = EFVInteractionSlot::Primary;
};

#undef UE_API
