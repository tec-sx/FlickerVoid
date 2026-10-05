#include "FVScannerSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FVScanDefinition.h"
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
