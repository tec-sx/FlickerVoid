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
	const UFVInteractableComponent* Found = Interactable.Get();
	if (!Found || !Targets.IsEmpty())
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

	CacheTargets();
	bIsHighlighted = bIsInFocus;

	const FFVInteractionHighlightSetup& Setup = bOverrideSetup
		? HighlightSetup
		: UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultHighlightSetup;

	for (int32 Index = 0; Index < Targets.Num(); ++Index)
	{
		UPrimitiveComponent* Target = Targets[Index];
		if (!IsValid(Target))
		{
			continue;
		}

		if (HighlightSetup.HighlightType == EFVHighlightType::PostProcessing)
		{
			Target->SetRenderCustomDepth(bIsInFocus ? true : CachedRenderCustomDepth[Index]);
			Target->SetCustomDepthStencilValue(bIsInFocus ? HighlightSetup.StencilID : CachedStencilValues[Index]);
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(Target))
		{
			Mesh->SetOverlayMaterial(bIsInFocus ? HighlightSetup.HighlightMaterial.Get() : nullptr);
		}
	}
}