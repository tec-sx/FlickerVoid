#include "Flow/Triggers/FVFlowTriggerBase.h"

#include "Components/BillboardComponent.h"
#include "Components/ShapeComponent.h"
#include "Flow/Components/FVFlowTriggerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFlowTriggerBase)

AFVFlowTriggerBase::AFVFlowTriggerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetHidden(true);
	SetCanBeDamaged(false);
	
	FlowTriggerComponent = CreateDefaultSubobject<UFVFlowTriggerComponent>(TEXT("FlowTriggerComponent"));
	TriggerZone = CreateDefaultSubobject<UShapeComponent>(TEXT("TriggerZone"));
	
	if (TriggerZone)
	{
		RootComponent = TriggerZone;
		TriggerZone->bHiddenInGame = false;
	}
	
#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> TriggerTextureObject;
			FName ID_Triggers;
			FText NAME_Triggers;
			FConstructorStatics()
				: TriggerTextureObject(TEXT("/Engine/EditorResources/S_Trigger"))
				, ID_Triggers(TEXT("Triggers"))
				, NAME_Triggers(NSLOCTEXT( "SpriteCategory", "Triggers", "Triggers" ))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.TriggerTextureObject.Get();
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = false;
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Triggers;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Triggers;
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif
}
