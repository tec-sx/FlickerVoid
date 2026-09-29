#include "Components/FVInteractableResponseComponent_Toggle.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableResponseComponent_Toggle)

void UFVInteractableResponseComponent_Toggle::BeginPlay()
{
	Super::BeginPlay();

	bIsOpen = bStartOpen;
	bIsLocked = bStartLocked;
}

void UFVInteractableResponseComponent_Toggle::SetLocked(const bool bLocked)
{
	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UFVInteractableResponseComponent_Toggle::ExecuteAction_Implementation(UFVInteractorComponent* Interactor)
{
	if (bIsLocked)
	{
		OnToggleBlocked(!bIsOpen);
		return;
	}

	bIsOpen = !bIsOpen;
	OnToggled(bIsOpen);
}
