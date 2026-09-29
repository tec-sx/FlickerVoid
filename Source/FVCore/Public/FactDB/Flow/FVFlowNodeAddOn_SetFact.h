#pragma once

#include "CoreMinimal.h"
#include "AddOns/FlowNodeAddOn.h"
#include "FactDB/FVFactTypes.h"
#include "FVFlowNodeAddOn_SetFact.generated.h"

/**
 * Writes a fact when its parent node is executed.
 *
 * Leave Input Pins empty to react to every input of the parent, or add pins to
 * scope the write to specific ones.
 */
UCLASS(MinimalApi, NotBlueprintable, meta = (DisplayName = "Set Fact"))
class UFVFlowNodeAddOn_SetFact : public UFlowNodeAddOn
{
	GENERATED_BODY()

public:
	UFVFlowNodeAddOn_SetFact();

	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "Fact", meta = (Categories = "Fact"))
	FGameplayTag Fact;

	UPROPERTY(EditAnywhere, Category = "Fact")
	int32 Value = 0;

	UPROPERTY(EditAnywhere, Category = "Fact")
	EFVFactValueChangeType ChangeType = EFVFactValueChangeType::Set;
};
