#include "Flow/Triggers/FVFlowTriggerBox.h"

#include "Engine/EngineTypes.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowTriggerBox)

static constexpr FColor FlowTriggerBaseColor(100, 255, 100, 255);
static const FName TriggerCollisionProfileName(TEXT("Trigger"));

AFVFlowTriggerBox::AFVFlowTriggerBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBoxComponent>(TEXT("TriggerZone")))
{
	UBoxComponent* BoxTriggerZone = CastChecked<UBoxComponent>(GetCollisionComponent());

	BoxTriggerZone->ShapeColor = FlowTriggerBaseColor;
	BoxTriggerZone->InitBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	BoxTriggerZone->SetCollisionProfileName(TriggerCollisionProfileName);
	
#if WITH_EDITORONLY_DATA
	if (UBillboardComponent* TriggerSpriteComponent = GetSpriteComponent())
	{
		TriggerSpriteComponent->SetupAttachment(BoxTriggerZone);
	}
#endif
}

#if WITH_EDITOR
void AFVFlowTriggerBox::EditorApplyScale(
	const FVector& DeltaScale, 
	const FVector* PivotLocation, 
	bool bAltDown,
	bool bShiftDown, 
	bool bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * ( AActor::bUsePercentageBasedScaling ? 500.0f : 5.0f );

	UBoxComponent * BoxComponent = CastChecked<UBoxComponent>(GetRootComponent());
	if (bCtrlDown)
	{
		// CTRL+Scaling modifies trigger collision height.  This is for convenience, so that height
		// can be changed without having to use the non-uniform scaling widget (which is
		// inaccessible with spacebar widget cycling).
		FVector Extent = BoxComponent->GetUnscaledBoxExtent() + FVector(0, 0, ModifiedScale.X);
		Extent.Z = FMath::Max(0.0f, Extent.Z);
		BoxComponent->SetBoxExtent(Extent);
	}
	else
	{
		FVector Extent = BoxComponent->GetUnscaledBoxExtent() + FVector(ModifiedScale.X, ModifiedScale.Y, ModifiedScale.Z);
		Extent.X = FMath::Max(0.0f, Extent.X);
		Extent.Y = FMath::Max(0.0f, Extent.Y);
		Extent.Z = FMath::Max(0.0f, Extent.Z);
		BoxComponent->SetBoxExtent(Extent);
	}
}
#endif
