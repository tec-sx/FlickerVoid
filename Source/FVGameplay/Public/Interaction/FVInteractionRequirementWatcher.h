#pragma once

#include "AttributeSet.h"
#include "Components/InteractionResponseComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "FVInteractionRequirementWatcher.generated.h"

class UInteractorComponent;

UCLASS(ClassGroup=(FlickerVoid), meta=(BlueprintSpawnableComponent))
class UFVInteractionRequirementWatcher final : public UInteractionResponceComponent
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BindResponses_Implementation(UInteractionResponseComponent* Response) override;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	FGameplayTagContainer WatchedTags;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	TArray<FGameplayAttribute> WatchedAttributes;

private:
	void RequestRefresh();

	TWeakObjectPtr<UInteractorComponent> Interactor;
	TArray<FDelegateHandle> TagHandles;
	TArray<TPair<FGameplayAttribute, FDelegateHandle>> AttributeHandles;
};
