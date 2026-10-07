#include "Scanner/FVScannableComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Conditions/FVConditionStatics.h"
#include "Scanner/FVScannerSubsystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScannableComponent)

UFVScannableComponent::UFVScannableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVScannableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetRevealed(false);
	Super::EndPlay(EndPlayReason);
}

bool UFVScannableComponent::CanBeScannedBy(AActor* Scanner) const
{
	return Definition && Definition->ScannableWhen.Evaluate(UFVConditionStatics::MakeContext(Scanner, GetOwner()));
}

TArray<FFVScanEntry> UFVScannableComponent::GetVisibleEntries(AActor* Scanner) const
{
	if (!Definition)
	{
		return {};
	}

	const FFVConditionContext Context = UFVConditionStatics::MakeContext(Scanner, GetOwner());
	return Definition->Entries.FilterByPredicate([&Context](const FFVScanEntry& Entry) { return Entry.VisibleWhen.Evaluate(Context); });
}

FVector UFVScannableComponent::GetScanLocation() const
{
	FVector Origin;
	FVector Extent;
	GetOwner()->GetActorBounds(true, Origin, Extent);
	return Origin;
}

void UFVScannableComponent::CompleteScan(AActor* Scanner)
{
	if (!Definition)
	{
		return;
	}

	if (!bScanned)
	{
		bScanned = true;
		Definition->OnScanned.Apply(UFVConditionStatics::MakeContext(Scanner, GetOwner()));
	}
	OnScanned.Broadcast(Scanner);
}

void UFVScannableComponent::SetRevealed(const bool bInRevealed)
{
	if (bRevealed == bInRevealed)
	{
		return;
	}
	bRevealed = bInRevealed;

	if (bRevealed)
	{
		const FFVScanCategory* Category = Definition ? GetDefault<UFVScannerSettings>()->FindCategory(Definition->Category) : nullptr;
		const int32 Stencil = Category ? Category->Stencil : 1;

		TArray<UPrimitiveComponent*> Primitives;
		GetOwner()->GetComponents(Primitives);
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			if (!Primitive->IsVisible())
			{
				continue;
			}
			SavedStates.Add({ Primitive, Primitive->bRenderCustomDepth != 0, Primitive->CustomDepthStencilValue });
			Primitive->SetRenderCustomDepth(true);
			Primitive->SetCustomDepthStencilValue(Stencil);
		}
	}
	else
	{
		for (const FPrimitiveState& State : SavedStates)
		{
			if (UPrimitiveComponent* Primitive = State.Primitive.Get())
			{
				Primitive->SetRenderCustomDepth(State.bRenderCustomDepth);
				Primitive->SetCustomDepthStencilValue(State.Stencil);
			}
		}
		SavedStates.Reset();
	}

	OnRevealChanged.Broadcast(bRevealed);
}
