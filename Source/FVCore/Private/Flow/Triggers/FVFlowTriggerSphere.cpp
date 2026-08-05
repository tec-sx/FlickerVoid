#include "Flow/Triggers/FVFlowTriggerSphere.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowTriggerSphere)

static constexpr FColor FlowTriggerBaseColor(100, 255, 100, 255);
static const FName TriggerCollisionProfileName(TEXT("Trigger"));

// Sets default values
AFVFlowTriggerSphere::AFVFlowTriggerSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USphereComponent>(TEXT("TriggerZone")))
{
	USphereComponent* SphereTriggerZone = CastChecked<USphereComponent>(GetCollisionComponent());

	SphereTriggerZone->ShapeColor = FlowTriggerBaseColor;
	SphereTriggerZone->InitSphereRadius(+200.0f);
	SphereTriggerZone->SetCollisionProfileName(TriggerCollisionProfileName);
	
#if WITH_EDITORONLY_DATA
	if (UBillboardComponent* TriggerSpriteComponent = GetSpriteComponent())
	{
		TriggerSpriteComponent->SetupAttachment(SphereTriggerZone);
	}
#endif
}

#if WITH_EDITOR
void AFVFlowTriggerSphere::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * ( AActor::bUsePercentageBasedScaling ? 500.0f : 5.0f );

	USphereComponent * SphereComponent = CastChecked<USphereComponent>(GetRootComponent());
	SphereComponent->SetSphereRadius(FMath::Max<float>(0.0f, SphereComponent->GetUnscaledSphereRadius() + ModifiedScale.X));
}
#endif
