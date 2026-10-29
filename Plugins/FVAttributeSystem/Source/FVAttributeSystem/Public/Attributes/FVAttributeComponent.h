#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVAttributeComponent.generated.h"

class UAbilitySystemComponent;
class UFVAttributeDefinition;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFVOnAttributeChanged, const UFVAttributeDefinition*, Attribute, float, OldValue, float, NewValue);

/**
 * Reports changes of the owner's defined attributes (on its ability system) by definition, and saves their
 * base values when the owner also has an FV Saveable Component. Values themselves live in GAS attribute sets.
 */
UCLASS(ClassGroup = FV, meta = (BlueprintSpawnableComponent))
class FVATTRIBUTESYSTEM_API UFVAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVAttributeComponent();

	static UFVAttributeComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Attributes", meta = (DisplayName = "Get FV Attribute Component"))
	static UFVAttributeComponent* Get(const AActor* Actor) { return Find(Actor); }

	UPROPERTY(BlueprintAssignable, Category = "FV|Attributes", meta = (ToolTip = "A defined attribute's current value changed."))
	FFVOnAttributeChanged OnAttributeChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void HandleActorDataLoaded();

	UPROPERTY(SaveGame)
	TMap<TObjectPtr<const UFVAttributeDefinition>, float> BaseValues;

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	TArray<TPair<const UFVAttributeDefinition*, FDelegateHandle>> ChangeHandles;
};
