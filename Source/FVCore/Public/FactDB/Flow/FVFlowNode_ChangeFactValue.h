#pragma once

#include "CoreMinimal.h"
#include "FVFlowNode_FactBase.h"
#include "FactDB/FVFactTypes.h"
#include "FVFlowNode_ChangeFactValue.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Change Fact"))
class FLICKERVOIDCORE_API UFVFlowNode_ChangeFactValue : public UFVFlowNode_FactBase
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_ChangeFactValue();
	
protected:
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
#endif
	
	UPROPERTY(EditAnywhere, Category = "Fact")
	int32 Value = 0;
	
	UPROPERTY(EditAnywhere, Category = "Fact")
	EFVFactValueChangeType ChangeType = EFVFactValueChangeType::Set;
};
