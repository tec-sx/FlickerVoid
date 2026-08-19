#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableComponent)

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(true);
	SetSphereRadius(400);
}


void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(GetOwner()))
	{
		OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnBeginOverlap);
		OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnEndOverlap);

		bIsInitialized = true;
	}
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsInitialized)
	{
		OnComponentBeginOverlap.RemoveDynamic(this, &UInteractableComponent::OnBeginOverlap);
		OnComponentEndOverlap.RemoveDynamic(this, &UInteractableComponent::OnEndOverlap);

		Super::EndPlay(EndPlayReason);
	}
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

FVector UInteractableComponent::GetAimProbeLocation() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	if (!AimProbeSocket.IsNone())
	{
		if (const USceneComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(AimProbeSocket))
			{
				return Mesh->GetSocketLocation(AimProbeSocket);
			}
		}
	}

	return Owner->GetActorLocation() + AimProbeOffset;
}

void UInteractableComponent::SetFocused(bool bFocused)
{
	if (bIsInitialized && bFocused != bIsInFocus)
	{
		bIsInFocus = bFocused;
	}
}

void UInteractableComponent::OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>();

	if (Interactor)
	{
		Interactor->RegisterCandidate(this);
	}
}

void UInteractableComponent::OnEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	UInteractorComponent* Interactor = OtherActor->FindComponentByClass<UInteractorComponent>();

	if (Interactor)
	{
		Interactor->UnregisterCandidate(this);
	}
}
