#include "Scanner/FVScannerComponent.h"

#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Scanner/FVScannableComponent.h"
#include "Scanner/FVScannerSubsystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScannerComponent)

UFVScannerComponent::UFVScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

UFVScannerComponent* UFVScannerComponent::Find(const AActor* Actor)
{
	return Actor != nullptr ? Actor->FindComponentByClass<UFVScannerComponent>() : nullptr;
}

void UFVScannerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetScanMode(false);
	Super::EndPlay(EndPlayReason);
}

void UFVScannerComponent::SetScanMode(const bool bActive)
{
	if (bScanMode == bActive)
	{
		return;
	}

	bScanMode = bActive;
	SetComponentTickEnabled(bActive);

	if (bActive)
	{
		UpdateAccumulator = UpdateInterval;
	}
	else
	{
		StopScan();
		SetFocus(nullptr);
		ClearRevealed();
	}

	if (UFVScannerSubsystem* Subsystem = UFVScannerSubsystem::Get(this))
	{
		Subsystem->SetScanModeActive(bActive);
	}
	OnScanModeChanged.Broadcast(bActive);
}

void UFVScannerComponent::StartScan()
{
	bScanning = bScanMode;
}

void UFVScannerComponent::StopScan()
{
	bScanning = false;
	if (Progress > 0.f)
	{
		Progress = 0.f;
		OnProgress.Broadcast(Progress);
	}
}

void UFVScannerComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float RealDelta = GetRealDeltaTime(DeltaTime);

	UpdateAccumulator += RealDelta;
	if (UpdateAccumulator >= UpdateInterval)
	{
		UpdateAccumulator = 0.f;
		UpdateTargets();
	}

	UFVScannableComponent* Target = Focus.Get();
	if (!bScanning || !Target || Target->WasScanned())
	{
		return;
	}

	const float Duration = Target->GetDefinition()->ScanDuration;
	Progress = Duration > 0.f ? FMath::Min(Progress + RealDelta / Duration, 1.f) : 1.f;
	OnProgress.Broadcast(Progress);

	if (Progress >= 1.f)
	{
		Target->CompleteScan(GetOwner());
		OnScanCompleted.Broadcast(Target);
		Progress = 0.f;
	}
}

void UFVScannerComponent::UpdateTargets()
{
	const UFVScannerSubsystem* Subsystem = UFVScannerSubsystem::Get(this);
	if (!Subsystem)
	{
		return;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	GetViewPoint(ViewLocation, ViewDirection);

	const float MinDot = FMath::Cos(FMath::DegreesToRadians(FocusAngle));
	UFVScannableComponent* BestFocus = nullptr;
	float BestDot = MinDot;
	TArray<TWeakObjectPtr<UFVScannableComponent>> NowRevealed;

	TArray<UFVScannableComponent*> Candidates;
	Subsystem->CollectScannables(ViewLocation, Range, Candidates);

	for (UFVScannableComponent* Scannable : Candidates)
	{
		if (Scannable->GetOwner() == GetOwner() || !Scannable->CanBeScannedBy(GetOwner()))
		{
			continue;
		}

		const FVector ToTarget = Scannable->GetScanLocation() - ViewLocation;

		Scannable->SetRevealed(true);
		NowRevealed.Add(Scannable);

		const float Dot = FVector::DotProduct(ViewDirection, ToTarget.GetSafeNormal());
		if (Dot > BestDot && (!bRequireLineOfSight || HasLineOfSight(ViewLocation, *Scannable)))
		{
			BestDot = Dot;
			BestFocus = Scannable;
		}
	}

	for (const TWeakObjectPtr<UFVScannableComponent>& Previous : Revealed)
	{
		if (Previous.IsValid() && !NowRevealed.Contains(Previous))
		{
			Previous->SetRevealed(false);
		}
	}
	Revealed = MoveTemp(NowRevealed);

	SetFocus(BestFocus);
}

void UFVScannerComponent::SetFocus(UFVScannableComponent* NewFocus)
{
	if (Focus.Get() == NewFocus)
	{
		return;
	}

	Focus = NewFocus;
	if (Progress > 0.f)
	{
		Progress = 0.f;
		OnProgress.Broadcast(Progress);
	}
	OnFocusChanged.Broadcast(NewFocus);
}

void UFVScannerComponent::ClearRevealed()
{
	for (const TWeakObjectPtr<UFVScannableComponent>& Scannable : Revealed)
	{
		if (Scannable.IsValid())
		{
			Scannable->SetRevealed(false);
		}
	}
	Revealed.Reset();
}

void UFVScannerComponent::GetViewPoint(FVector& OutLocation, FVector& OutDirection) const
{
	FRotator Rotation;
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (const AController* Controller = Pawn ? Pawn->GetController() : nullptr)
	{
		Controller->GetPlayerViewPoint(OutLocation, Rotation);
	}
	else
	{
		GetOwner()->GetActorEyesViewPoint(OutLocation, Rotation);
	}
	OutDirection = Rotation.Vector();
}

bool UFVScannerComponent::HasLineOfSight(const FVector& From, const UFVScannableComponent& Scannable) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(FVScannerLineOfSight), false, GetOwner());
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, From, Scannable.GetScanLocation(), TraceChannel, Params))
	{
		return true;
	}
	return Hit.GetActor() == Scannable.GetOwner();
}

float UFVScannerComponent::GetRealDeltaTime(const float DeltaTime) const
{
	const float Dilation = UGameplayStatics::GetGlobalTimeDilation(this) * GetOwner()->CustomTimeDilation;
	return DeltaTime / FMath::Max(Dilation, KINDA_SMALL_NUMBER);
}
