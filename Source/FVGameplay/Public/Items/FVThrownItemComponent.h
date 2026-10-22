#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/FVGameplayItemFragments.h"
#include "FVThrownItemComponent.generated.h"

class UPrimitiveComponent;
struct FHitResult;

/** Added to a thrown item by the throw ability: its first hit makes noise for AI hearing and the game (Message.Item.Noise), plays the impact sound and breaks a fragile item. */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVThrownItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVThrownItemComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "FV|Item|Throw", meta = (ToolTip = "Starts listening for the first hit of the owner's root body; the throwable fragment gives the noise and the impact."))
	void Arm(AActor* InThrower, const FFVItemFragment_Throwable& InThrowable);

private:
	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> Body;

	TWeakObjectPtr<AActor> Thrower;
	FFVItemFragment_Throwable Throwable;
};
