// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "StateTree/FVStateTreeEvent.h"
#include "FVSTE_Perception.generated.h"

class UBoxComponent;
struct FAIStimulus;
class AFVAICharacterController;
class AFVAICharacter;
class UFVStateTreeAIComponent;
class AAIController;

UCLASS(DisplayName="Perception Evaluator", Category="Flicker Void")
class FLICKERVOIDAI_API UFVSTE_Perception : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context)
	TObjectPtr<AFVAICharacter> OwnerCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context)
	TWeakObjectPtr<AFVAICharacterController> AIController;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Output)
	TObjectPtr<AActor> AcquiredTarget = nullptr;
	
#if WITH_EDITOR
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
	{
		return NSLOCTEXT("STE_Perception", "PerceptionDesc", "Monitors the Context Actor's AI perceptions");
	};
#endif
	
protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) override;
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
    void OnSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
    void OnSightStimulusForgotten(AActor* Actor);
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
    void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
    void OnHearingStimulusForgotten(AActor* Actor);
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
    void OnDamageStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
	void OnEnterInteractionRange(AActor* Actor, const FAIStimulus& Stimulus);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Perception")
	void OnExitInteractionRange(AActor* Actor);
	
private:
    UFUNCTION()
    void HandleSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);

    UFUNCTION()
    void HandleSightStimulusForgotten(AActor* Actor);

    UFUNCTION()
    void HandleHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

    UFUNCTION()
    void HandleHearingStimulusForgotten(AActor* Actor);

    UFUNCTION()
    void HandleDamageStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	
	UFUNCTION()
	void HandleEnterInteractionZone(AActor* Actor);
	
	UFUNCTION()
	void HandleExitInteractionZone(AActor* Actor);

    /** Sends a StateTree event with the specified tag */
    void SendEvent(const EFVStateTreeEvent InEvent);
	
    UPROPERTY()
    TObjectPtr<UFVStateTreeAIComponent> StateTreeComp;
	
	UPROPERTY()
	TObjectPtr<UBoxComponent> InteractionZone;
	
    EFVStateTreeEvent LastEvent = EFVStateTreeEvent::Idle;
};
