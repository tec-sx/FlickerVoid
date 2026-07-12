// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "FVSTC_CompareGameplayTags.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Compare Gameplay Tags", Category="Flicker Void")
class FLICKERVOIDCORE_API UFVSTC_CompareGameplayTags : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()
	
public:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
	
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
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FGameplayTag ConditionTag;

	/** The second gameplay tag to compare. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	bool ExactMatch = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	bool Invert = false;
};
