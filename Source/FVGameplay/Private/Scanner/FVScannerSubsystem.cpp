#include "Scanner/FVScannerSubsystem.h"

#include "Scanner/FVScannableComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Scanner/FVScanDefinition.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScannerSubsystem)

UFVScannerSubsystem* UFVScannerSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVScannerSubsystem>() : nullptr;
}

void UFVScannerSubsystem::SetScanModeActive(const bool bActive)
{
	if (bScanModeActive == bActive)
	{
		return;
	}

	bScanModeActive = bActive;
	UGameplayStatics::SetGlobalTimeDilation(this, bActive ? GetDefault<UFVScannerSettings>()->ScanModeTimeDilation : 1.f);
	OnScanModeChanged.Broadcast(bActive);
}

void UFVScannerSubsystem::SetInRange(UFVScannableComponent* Scannable, const bool bInRange)
{
	if (bInRange)
	{
		InRange.AddUnique(Scannable);
	}
	else
	{
		InRange.RemoveSingleSwap(Scannable, EAllowShrinking::No);
	}
}

void UFVScannerSubsystem::CollectScannables(TArray<UFVScannableComponent*>& OutScannables) const
{
	for (UFVScannableComponent* Scannable : InRange)
	{
		if (IsValid(Scannable))
		{
			OutScannables.Add(Scannable);
		}
	}
}
