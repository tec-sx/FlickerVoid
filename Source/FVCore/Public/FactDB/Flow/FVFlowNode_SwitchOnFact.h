#pragma once

#include "CoreMinimal.h"
#include "FVFlowNode_FactBase.h"
#include "FVFlowNode_SwitchOnFact.generated.h"

/**
 * Routes execution based on the current value of a single fact.
 *
 * Each configured value gets its own output pin; anything else (including an
 * undefined fact) takes Default.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Switch On Fact"))
class FLICKERVOIDCORE_API UFVFlowNode_SwitchOnFact : public UFVFlowNode_FactBase
{
	GENERATED_BODY()

public:
	UFVFlowNode_SwitchOnFact();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual bool SupportsContextPins() const override { return true; }
	virtual TArray<FFlowPin> GetContextOutputs() const override;
#endif

protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
	virtual FString GetNodeDescription() const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Fact")
	TArray<int32> Values;

	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bUndefinedTakesDefault = true;

	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bUseEnumForDisplay = false;

	UPROPERTY(EditAnywhere, Category = "Fact", meta = (EditCondition = "bUseEnumForDisplay", GetOptions = "GetRegisteredFactEnumNames"))
	FName ValueEnumName;

	UFUNCTION()
	static TArray<FName> GetRegisteredFactEnumNames();

private:
	static FName MakeValuePinName(int32 Value);

#if WITH_EDITOR
	FText MakeValuePinFriendlyName(int32 Value) const;
#endif
};
