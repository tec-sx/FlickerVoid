#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVTagLibrary.generated.h"

/**
 * The one way to ask what gameplay tags an actor has: merges every tag owning component on it
 * (the ability system component, UFVTagComponent), so systems don't depend on where a tag was granted.
 */
UCLASS()
class FVCORERUNTIME_API UFVTagLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Tags", meta = (ToolTip = "Every gameplay tag the actor's tag owning components hold."))
	static FGameplayTagContainer GetActorTags(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Tags", meta = (ToolTip = "Whether the actor has Tag or a child of it."))
	static bool ActorHasTag(const AActor* Actor, FGameplayTag Tag);

	UFUNCTION(BlueprintPure, Category = "FV|Tags", meta = (ToolTip = "Whether the actor has none of Blocked and all (or any) of Required. Empty Required passes."))
	static bool ActorMatchesTags(const AActor* Actor, const FGameplayTagContainer& Required, const FGameplayTagContainer& Blocked, bool bRequireAll = true);
};
