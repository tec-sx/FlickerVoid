#include "Components/FVInteractableHighlightComponent.h"

#include "Components/FVInteractableComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableHighlightComponent)

void UFVInteractableHighlightComponent::BeginPlay()
{
	Super::BeginPlay();

	UFVInteractableComponent* Found = GetOwner()->FindComponentByClass<UFVInteractableComponent>();
	if (!ensureMsgf(Found, TEXT("%s: highlight requires a UFVInteractableComponent."), *GetNameSafe(GetOwner())))
	{
		return;
	}

	Interactable = Found;
	Found->InteractorFound.AddDynamic(this, &ThisClass::OnInteractorFound);
	Found->InteractorLost.AddDynamic(this, &ThisClass::OnInteractorLost);
}

void UFVInteractableHighlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RenderHighlight(false);

	if (UFVInteractableComponent* Found = Interactable.Get())
	{
		Found->InteractorFound.RemoveDynamic(this, &ThisClass::OnInteractorFound);
		Found->InteractorLost.RemoveDynamic(this, &ThisClass::OnInteractorLost);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInteractableHighlightComponent::OnInteractorFound(UFVInteractorComponent* Interactor)
{
	RenderHighlight(true);
}

void UFVInteractableHighlightComponent::OnInteractorLost(UFVInteractorComponent* Interactor)
{
	RenderHighlight(false);
}

void UFVInteractableHighlightComponent::CacheTargets()
{
	Targets.Reset();
	CachedOverlays.Reset();
	CachedRenderCustomDepth.Reset();
	CachedStencilValues.Reset();

	const UFVInteractableComponent* Found = Interactable.Get();
	if (!Found)
	{
		return;
	}

	for (UPrimitiveComponent* Target : Found->GetDetectablePrimitives())
	{
		if (!IsValid(Target))
		{
			continue;
		}

		const UMeshComponent* Mesh = Cast<UMeshComponent>(Target);
		Targets.Add(Target);
		CachedOverlays.Add(Mesh ? Mesh->GetOverlayMaterial() : nullptr);
		CachedRenderCustomDepth.Add(Target->bRenderCustomDepth);
		CachedStencilValues.Add(Target->CustomDepthStencilValue);
	}
}

void UFVInteractableHighlightComponent::RenderHighlight(const bool bIsInFocus)
{
	if (bIsHighlighted == bIsInFocus)
	{
		return;
	}

	bIsHighlighted = bIsInFocus;
	if (bIsInFocus)
	{
		ApplyHighlight();
	}
	else
	{
		RestoreTargets();
	}
}

void UFVInteractableHighlightComponent::ApplyHighlight()
{
	// Cached on every focus, so primitives added later are included and the restored values are current.
	CacheTargets();

	const FFVInteractionHighlightSetup& Setup = bOverrideSetup
		? HighlightSetup
		: UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultHighlightSetup;

	for (UPrimitiveComponent* Target : Targets)
	{
		if (Setup.HighlightType == EFVHighlightType::PostProcessing)
		{
			Target->SetRenderCustomDepth(true);
			Target->SetCustomDepthStencilValue(Setup.StencilID);
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(Target))
		{
			UMaterialInterface* Material = Setup.HighlightMaterial.Get();
			Mesh->SetOverlayMaterial(Material);
		}
	}
}

void UFVInteractableHighlightComponent::RestoreTargets()
{
	for (int32 Index = 0; Index < Targets.Num(); ++Index)
	{
		UPrimitiveComponent* Target = Targets[Index];
		if (!IsValid(Target))
		{
			continue;
		}

		Target->SetRenderCustomDepth(CachedRenderCustomDepth[Index]);
		Target->SetCustomDepthStencilValue(CachedStencilValues[Index]);
		if (UMeshComponent* Mesh = Cast<UMeshComponent>(Target))
		{
			Mesh->SetOverlayMaterial(CachedOverlays[Index]);
		}
	}
}
