#include "Components/Response_Highlight.h"

#include "Components/InteractableComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_Highlight)

void UResponse_Highlight::BindInteractableResponses_Implementation(UInteractableComponent* Interactable)
{
	const FInteractableSettings& Defaults = UFVInteractionSystemSettings::Get().InteractableBaseSettings;

	if (SetupType == EHighlightSetupType::Default)
	{
		SetupType = Defaults.DefaultHighlightSetupType;
	}

	if (!HighlightSetup.HighlightMaterial)
	{
		HighlightSetup = Defaults.DefaultHighlightSetup;
	}

	CollectTargets();

	Interactable->FocusStateChanged.AddDynamic(this, &UResponse_Highlight::HandleFocusStateChanged);
}

void UResponse_Highlight::CollectTargets()
{
	HighlightTargets.Reset();

	AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	if (SetupType == EHighlightSetupType::Quick)
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
}

void UResponse_Highlight::HandleFocusStateChanged(bool bInFocus, UInteractorComponent* Interactor)
{
	ApplyHighlight(bInFocus);
}

void UResponse_Highlight::ApplyHighlight(bool bEnabled)
{
	for (int32 Index = 0; Index < HighlightTargets.Num(); ++Index)
	{
		UPrimitiveComponent* Target = HighlightTargets[Index];
		if (!IsValid(Target))
		{
			continue;
		}

		if (HighlightSetup.HighlightType == EHighlightType::PostProcessing)
		{
			Target->SetRenderCustomDepth(bEnabled ? true : CachedRenderCustomDepth[Index]);
			Target->SetCustomDepthStencilValue(bEnabled ? HighlightSetup.StencilID : CachedStencilValues[Index]);
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(Target))
		{
			Mesh->SetOverlayMaterial(bEnabled ? HighlightSetup.HighlightMaterial.Get() : nullptr);
		}
	}
}
