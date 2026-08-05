#include "Interactions/FVInteractionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionComponent)

FFVPlayerInInteractionEvent UFVInteractionComponent::OnPlayerEnter;
FFVPlayerInInteractionEvent UFVInteractionComponent::OnPlayerExit;

// Sets default values for this component's properties
UFVInteractionComponent::UFVInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Distance(300.0f)
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	SetUsingAbsoluteScale(true);
	ArrowColor = FColor::Orange;
	ArrowSize = 0.5f;
}


// Called when the game starts
void UFVInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnabled)
	{
		Enable();
	}
}


// Called every frame
void UFVInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bConditionsMet = false;
	if (CameraManager.IsValid())
	{
		const FVector DistanceToCamera = GetComponentLocation() - CameraManager->GetCameraLocation();
		bConditionsMet = DistanceToCamera.Size() < Distance;
	}

	if (bConditionsMet)
	{
		if (!bCanInteract)
		{
			bCanInteract = true;
			OnPlayerEnter.Broadcast(this);
		}
	}
	else if (bCanInteract)
	{
		bCanInteract = false;
		OnPlayerExit.Broadcast(this);
	}
}

void UFVInteractionComponent::Enable()
{
	if (const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		bEnabled = true;

		CameraManager = PlayerController->PlayerCameraManager;
		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UFVInteractionComponent::Disable()
{
	if (bCanInteract)
	{
		bCanInteract = false;
		OnPlayerExit.Broadcast(this);
	}

	bEnabled = false;

	PrimaryComponentTick.SetTickFunctionEnable(false);
	CameraManager = nullptr;
}

