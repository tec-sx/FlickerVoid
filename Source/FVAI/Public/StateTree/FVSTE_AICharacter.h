// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "FVSTE_AICharacter.generated.h"

class AFVAICharacterController;
class AFVAICharacter;
/**
 * 
 */
UCLASS()
class FLICKERVOIDAI_API UFVSTE_AICharacter : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()
	
public:
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AFVAICharacter> Character;

	/** Pointer to the bound controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AFVAICharacterController> AIController;

	/** Out: the actor’s current health. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output)
	float CurrentHealth = 0.f;
	
#if WITH_EDITOR

	/**
	 * Gets the description of the condition for editor display.
	 * @param ID The unique identifier for the node.
	 * @param InstanceDataView The data view for the instance.
	 * @param BindingLookup The binding lookup for the state tree.
	 * @param Formatting The formatting options for the description.
	 * @return The formatted description text.
	*/
	virtual FText GetDescription(
		const FGuid& ID,
		FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const {
		return NSLOCTEXT(
			"FVSTE_AICharacter",
			"EnemyCharacterEvaluatorDesc",
			"Monitors the (Enemy) Context Actor"
		);
	};

#endif
};
