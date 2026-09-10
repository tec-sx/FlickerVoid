#include "Components/Response_Highlight.h"

#include "Components/InteractableComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystemSettings.h"
#include "Components/InteractionSignalComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(Response_Highlight)

void UResponse_Highlight::BindSignals_Implementation(UInteractionSignalComponent* Signal)
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

	HighlightTargets.Reset();

	const AActor* OwningActor = GetOwner();
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

	Signal->FocusChanged.AddDynamic(this, &UResponse_Highlight::OnFocusStateChanged);
}

void UResponse_Highlight::UnbindSignals_Implementation(UInteractionSignalComponent* Signal)
{
	Signal->FocusChanged.RemoveDynamic(this, &UResponse_Highlight::OnFocusStateChanged);
	
	CachedRenderCustomDepth.Reset();
	CachedStencilValues.Reset();
	HighlightTargets.Reset();
}

void UResponse_Highlight::OnFocusStateChanged(UInteractableComponent* Interactable)
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
			Target->SetRenderCustomDepth(Interactable->IsInFocus() ? true : CachedRenderCustomDepth[Index]);
			Target->SetCustomDepthStencilValue(Interactable->IsInFocus() ? HighlightSetup.StencilID : CachedStencilValues[Index]);
		}
		else if (UMeshComponent* Mesh = Cast<UMeshComponent>(Target))
		{
			Mesh->SetOverlayMaterial(Interactable->IsInFocus() ? HighlightSetup.HighlightMaterial.Get() : nullptr);
		}
	}
}
