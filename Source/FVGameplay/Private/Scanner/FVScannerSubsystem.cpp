#include "Scanner/FVScannerSubsystem.h"

#include "Components/FVInteractableComponent.h"
#include "Scanner/FVScannableComponent.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"

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

void UFVScannerSubsystem::CollectScannables(const FVector& Origin, const float Range, TArray<UFVScannableComponent*>& OutScannables) const
{
	const UFVInteractionRegistrySubsystem* Registry = GetWorld() != nullptr ? GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>() : nullptr;
	if (Registry == nullptr)
	{
		return;
	}

	const float RangeSquared = FMath::Square(Range);

	for (const TObjectPtr<UFVInteractableComponent>& Interactable : Registry->GetAllInteractables())
	{
		const AActor* Owner = Interactable != nullptr ? Interactable->GetOwner() : nullptr;
		UFVScannableComponent* Scannable = Owner != nullptr ? Owner->FindComponentByClass<UFVScannableComponent>() : nullptr;

		if (Scannable != nullptr && FVector::DistSquared(Scannable->GetScanLocation(), Origin) <= RangeSquared)
		{
			OutScannables.Add(Scannable);
		}
	}
}
