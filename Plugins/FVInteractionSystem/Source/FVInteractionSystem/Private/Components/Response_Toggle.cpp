#include "Components/Response_Toggle.h"

#include "Components/InteractableComponent.h"
#include "Components/InteractionSignalComponent.h"
#include "Components/InteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_Toggle)

void UResponse_Toggle::BeginPlay()
{
	Super::BeginPlay();
	
	bIsOn = bInitialState;
	bIsLocked = bStartsLocked;
}

void UResponse_Toggle::BindSignals_Implementation(UInteractionSignalComponent* Signal)
{
	Signal->InteractionRequested.AddDynamic(this, &UResponse_Toggle::OnInteractionRequested);
}

void UResponse_Toggle::UnbindSignals_Implementation(UInteractionSignalComponent* Signal)
{
	Signal->InteractionRequested.RemoveDynamic(this, &UResponse_Toggle::OnInteractionRequested);
}

void UResponse_Toggle::SetLocked(const bool bLocked)
{
	bIsLocked = bLocked;
	OnLockChanged(bIsLocked);
}

void UResponse_Toggle::OnInteractionRequested(const FInteractionCommit& Commit)
{
	if (Commit.ActionTag.MatchesTagExact(ToggleActionTag))
	{
		if (bIsLocked)
		{
			OnToggleBlocked(!bIsOn);
			return;
		}

		bIsOn = !bIsOn;
		OnToggled(bIsOn);
	}
}
