#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "FVTagComponent.generated.h"

/**
 * Gameplay tag store for actors without an ability system component, such as NPCs and world actors.
 * UFVTagLibrary reads it together with any other tag owning component on the actor.
 */
UCLASS(ClassGroup = (FlickerVoid), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVTagComponent : public UActorComponent, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	UFVTagComponent();

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(BlueprintCallable, Category = "FV|Tags")
	void AddTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "FV|Tags")
	void AddTags(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintCallable, Category = "FV|Tags")
	void RemoveTag(FGameplayTag Tag);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags", meta = (ToolTip = "Tags the actor owns, starting with these."))
	FGameplayTagContainer Tags;
};
