// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "NativeGameplayTags.h"
#include "FVFlowNode_FactBase.generated.h"

namespace FlowNodeStyle
{
	FLICKERVOIDCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fact);
}

UCLASS(Abstract, NotBlueprintable, HideCategories=("Internal"))
class FLICKERVOIDCORE_API UFVFlowNode_FactBase : public UFlowNode
{
	GENERATED_BODY()
public:
	UFVFlowNode_FactBase();

	virtual EFlowAddOnAcceptResult AcceptFlowNodeAddOnChild_Implementation(
		const UFlowNodeAddOn* AddOnTemplate,
		const TArray<UFlowNodeAddOn*>& AdditionalAddOnsToAssumeAreChildren) const override;

protected:
	/** True when every predicate AddOn attached to this node passes (also true when there are none). */
	bool ArePredicateAddOnsSatisfied() const;

#if WITH_EDITOR
	virtual EDataValidationResult ValidateNode() override;
#endif
	
	// Intentionally scoped to the whole "Fact" tree. Categories cannot be overridden per
	// subclass, so nodes that need a narrower picker declare their own tag property instead.
	UPROPERTY(EditAnywhere, Category = "Fact", meta=(Categories = "Fact"))
	FGameplayTag Fact;
};
