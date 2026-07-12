#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "FVSTE_Interaction.generated.h"

class AFVAICharacterController;
class AFVAICharacter;

UCLASS()
class FLICKERVOIDAI_API UFVSTE_Interaction : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()
	
// public:
// 	UFVSTE_Interaction();
// 	
// 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context)
// 	TObjectPtr<AFVAICharacter> OwnerCharacter;
// 	
// 	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context)
// 	TWeakObjectPtr<AFVAICharacterController> AIController;
// 	
// #if WITH_EDITOR
// 	virtual FText GetDescription(
// 		const FGuid& ID,
// 		FStateTreeDataView InstanceDataView,
// 		const IStateTreeBindingLookup& BindingLookup,
// 		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
// 	{
// 		return NSLOCTEXT("STE_Interaction", "InteractionDesc", "Monitors the Context Actor's Interaction status");
// 	};
// #endif
// 	
// protected:
// 	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
// 	virtual void TreeStop(FStateTreeExecutionContext& Context) override;
// 	
// private:
// 	UFUNCTION()
// 	void OnInteractionSphereEnter(
// 		UPrimitiveComponent* OverlappedComp,
// 		AActor* OtherActor,
// 		UPrimitiveComponent* OtherComp,
// 		int32 OtherBodyIndex,
// 		bool bFromSweep,
// 		const FHitResult& SweepResult);
// 	
// 	UFUNCTION()
// 	void OnInteractionSphereExit(
// 		UPrimitiveComponent* OverlappedComp,
// 		AActor* OtherActor,
// 		UPrimitiveComponent* OtherComp,
// 		int32 OtherBodyIndex);
};
