#pragma once

#include "CoreMinimal.h"
#include "FVFlowNode_FactBase.h"
#include "FVFlowNode_SwitchOnFact.generated.h"

/**
 * Routes execution based on the current value of a single fact.
 *
 * Each configured value gets its own output pin; anything else (including an
 * undefined fact) takes Default. Intended for quest Stage routing, where the
 * values come from EFVQuestStage.
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

	/** Values to branch on. Duplicates are ignored; unmatched values take Default. */
	UPROPERTY(EditAnywhere, Category = "Fact")
	TArray<int32> Values;

	/** When set, an undefined fact takes Default instead of matching a configured 0. */
	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bUndefinedTakesDefault = true;

	/** Label the value pins with enumerator names. Pin identity stays numeric, so renaming an enumerator never breaks a connection. */
	UPROPERTY(EditAnywhere, Category = "Fact")
	bool bUseEnumForDisplay = false;

	/** Enum used to label the value pins, typically EFVQuestStage. */
	UPROPERTY(EditAnywhere, Category = "Fact", meta = (EditCondition = "bUseEnumForDisplay"))
	TObjectPtr<UEnum> ValueEnum;

private:
	static FName MakeValuePinName(int32 Value);

#if WITH_EDITOR
	FText MakeValuePinFriendlyName(int32 Value) const;
#endif
};
