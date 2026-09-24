#include "Components/FVInteractableResponseComponent_Highlight.h"

#include "Components/FVInteractableComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableResponseComponent_Highlight)

void UFVInteractableResponseComponent_Highlight::BindEvents_Implementation(UFVInteractableComponent* Interactable)
{
	const FFVInteractableSettings& Defaults = UFVInteractionSystemSettings::Get().InteractableBaseSettings;

	if (SetupType == EFVHighlightSetupType::Default)
	{
		SetupType = Defaults.DefaultHighlightSetupType;
	}

	if (!HighlightSetup.HighlightMaterial)
	{
		HighlightSetup = Defaults.DefaultHighlightSetup;
	}

	HighlightTargets.Reset();

	const AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	if (SetupType == EFVHighlightSetupType::Quick)
	{
		OwningActor->GetComponents<UPrimitiveComponent>(HighlightTargets);
	}
	else if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(OwningActor->GetRootComponent()))
	{
		HighlightTargets.Add(Root);
	}

	CachedRenderCustomDepth.Reset();
	CachedStencilValues.Reset();

	for (const UPrimitiveComponent* Target : HighlightTargets)
	{
		CachedRenderCustomDepth.Add(Target->bRenderCustomDepth);
		CachedStencilValues.Add(Target->CustomDepthStencilValue);
	}

	Interactable->InteractorFound.AddDynamic(this, &UFVInteractableResponseComponent_Highlight::OnInteractorFound);
	Interactable->InteractorLost.AddDynamic(this, &UFVInteractableResponseComponent_Highlight::OnInteractorLost);
}

void UFVInteractableResponseComponent_Highlight::UnbindEvents_Implementation(UFVInteractableComponent* Interactable)
{
	Interactable->InteractorFound.RemoveDynamic(this, &UFVInteractableResponseComponent_Highlight::OnInteractorFound);
	Interactable->InteractorLost.RemoveDynamic(this, &UFVInteractableResponseComponent_Highlight::OnInteractorLost);
	
	CachedRenderCustomDepth.Reset();
	CachedStencilValues.Reset();
	HighlightTargets.Reset();
}

void UFVInteractableResponseComponent_Highlight::OnInteractorFound(UFVInteractorComponent* Interactor)
{
	RenderHighlight(true);
}

void UFVInteractableResponseComponent_Highlight::OnInteractorLost(UFVInteractorComponent* Interactor)
{
	RenderHighlight(false);
}

void UFVInteractableResponseComponent_Highlight::RenderHighlight(const bool bIsInFocus)
{
	for (int32 Index = 0; Index < HighlightTargets.Num(); ++Index)
	{
		UPrimitiveComponent* Target = HighlightTargets[Index];
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