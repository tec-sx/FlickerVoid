#include "HUD/FVHUD.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Layout/FVUILayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVHUD)

static UFVUIManagerSubsystem* GetUIManager(const APlayerController* PC)
{
	const ULocalPlayer* LocalPlayer = PC ? PC->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UFVUIManagerSubsystem>() : nullptr;
}

UFVUILayout* AFVHUD::GetLayout() const
{
	const UFVUIManagerSubsystem* UIManager = GetUIManager(GetOwningPlayerController());
	return UIManager ? UIManager->GetLayout() : nullptr;
}

void AFVHUD::BeginPlay()
{
	Super::BeginPlay();

	if (UFVUIManagerSubsystem* UIManager = GetUIManager(GetOwningPlayerController()))
	{
		if (UFVUILayout* Layout = UIManager->EnsureLayout())
		{
			OnLayoutReady(Layout);
		}
	}
}

void AFVHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVUIManagerSubsystem* UIManager = GetUIManager(GetOwningPlayerController()))
	{
		UIManager->ReleaseLayout();
	}

	Super::EndPlay(EndPlayReason);
}
