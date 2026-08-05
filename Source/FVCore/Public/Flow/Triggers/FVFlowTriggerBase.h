#pragma once

#include "GameFramework/Actor.h"
#include "FVFlowTriggerBase.generated.h"

class UFVFlowTriggerComponent;

UCLASS()
class FLICKERVOIDCORE_API AFVFlowTriggerBase : public AActor
{
	GENERATED_BODY()
public:
	explicit AFVFlowTriggerBase(const FObjectInitializer& ObjectInitializer);
	
private:
	UPROPERTY(Category = FlowTriggerBase, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFVFlowTriggerComponent> FlowTriggerComponent;
	
	UPROPERTY(Category = FlowTriggerBase, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UShapeComponent> TriggerZone;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

public:
	UShapeComponent* GetCollisionComponent() const { return TriggerZone; }

#if WITH_EDITORONLY_DATA
	UBillboardComponent* GetSpriteComponent() const { return SpriteComponent; }
#endif
};
