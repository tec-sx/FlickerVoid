#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "FVAICharacterStateTreeSchema.generated.h"

UCLASS()
class FLICKERVOIDAI_API UFVAICharacterStateTreeSchema : public UStateTreeAIComponentSchema
{
	GENERATED_BODY()
	
public:
	UFVAICharacterStateTreeSchema();
	
	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override;
	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
	
private:
	static TArray<FStateTreeExternalDataDesc> ContextDataDescriptors;
};
