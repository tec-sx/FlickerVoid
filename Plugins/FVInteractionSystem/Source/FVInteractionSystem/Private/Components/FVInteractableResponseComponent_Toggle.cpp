#include "Components/FVInteractableResponseComponent_Toggle.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Components/FVLockComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableResponseComponent_Toggle)

void UFVInteractableResponseComponent_Toggle::BeginPlay()
{
	Super::BeginPlay();

	bIsOpen = bStartOpen;
	bIsLocked = bStartLocked;

	LockComponent = GetOwner()->FindComponentByClass<UFVLockComponent>();
	if (LockComponent)
	{
		LockComponent->OnLockStateChanged.AddDynamic(this, &ThisClass::HandleLockStateChanged);
	}
}

bool UFVInteractableResponseComponent_Toggle::IsLocked() const
{
	return LockComponent ? LockComponent->IsLocked() : bIsLocked;
}

void UFVInteractableResponseComponent_Toggle::SetLocked(const bool bLocked)
{
	if (LockComponent)
	{
		// The lock component broadcasts back into HandleLockStateChanged.
		LockComponent->SetLocked(bLocked);
		return;
	}

	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UFVInteractableResponseComponent_Toggle::HandleLockStateChanged(const bool bLocked, AActor* Instigator)
{
	OnLockChanged(bLocked);
}

void UFVInteractableResponseComponent_Toggle::ExecuteAction_Implementation(UFVInteractorComponent* Interactor)
{
	if (IsLocked())
	{
		OnToggleBlocked(!bIsOpen);
		return;
	}

	bIsOpen = !bIsOpen;
	OnToggled(bIsOpen);
}
