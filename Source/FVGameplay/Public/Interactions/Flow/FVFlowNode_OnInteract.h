// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Nodes/Actor/FlowNode_ComponentObserver.h"
#include "FVFlowNode_OnInteract.generated.h"

class UFlowComponent;
class UFVInteractionComponent;

UCLASS(NotBlueprintable, meta = (DisplayName = "On Interact"))
class FLICKERVOIDGAMEPLAY_API UFVFlowNode_OnInteract : public UFlowNode_ComponentObserver
{
	GENERATED_BODY()
	
protected:
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UFVInteractionComponent>> ObservedInteractions;
	
	virtual void ObserveActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;
	virtual void ForgetActor(TWeakObjectPtr<AActor> Actor, TWeakObjectPtr<UFlowComponent> Component) override;
	virtual void Cleanup() override;
};
