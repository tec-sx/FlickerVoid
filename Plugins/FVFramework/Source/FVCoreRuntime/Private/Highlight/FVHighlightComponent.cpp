#include "Highlight/FVHighlightComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVHighlightComponent)

UFVHighlightComponent::UFVHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVHighlightComponent* UFVHighlightComponent::FindOrAdd(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (UFVHighlightComponent* Found = Actor->FindComponentByClass<UFVHighlightComponent>())
	{
		return Found;
	}

	UFVHighlightComponent* Added = NewObject<UFVHighlightComponent>(Actor);
	Added->RegisterComponent();
	return Added;
}

void UFVHighlightComponent::PushHighlight(const FName Source, const int32 Stencil, const int32 Priority, const TArray<UPrimitiveComponent*>& Primitives)
{
	Requests.RemoveAll([Source](const FRequest& Request) { return Request.Source == Source; });

	FRequest& Request = Requests.AddDefaulted_GetRef();
	Request.Source = Source;
	Request.Stencil = Stencil;
	Request.Priority = Priority;
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Request.Primitives.Add(Primitive);
	}

	Refresh();
}

void UFVHighlightComponent::PopHighlight(const FName Source)
{
	const int32 Removed = Requests.RemoveAll([Source](const FRequest& Request) { return Request.Source == Source; });
	if (Removed > 0)
	{
		Refresh();
	}
}

void UFVHighlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Requests.Reset();
	Refresh();
	Super::EndPlay(EndPlayReason);
}

const UFVHighlightComponent::FRequest* UFVHighlightComponent::FindTopRequest(const UPrimitiveComponent* Primitive) const
{
	const FRequest* Top = nullptr;
	for (const FRequest& Request : Requests)
	{
		const bool bCovers = Request.Primitives.ContainsByPredicate([Primitive](const TWeakObjectPtr<UPrimitiveComponent>& It) { return It.Get() == Primitive; });
		if (bCovers && (!Top || Request.Priority >= Top->Priority))
		{
			Top = &Request;
		}
	}
	return Top;
}

void UFVHighlightComponent::Refresh()
{
	TSet<UPrimitiveComponent*> Covered;
	for (const FRequest& Request : Requests)
	{
		for (const TWeakObjectPtr<UPrimitiveComponent>& Weak : Request.Primitives)
		{
			if (UPrimitiveComponent* Primitive = Weak.Get())
			{
				Covered.Add(Primitive);
			}
		}
	}

	for (auto It = Originals.CreateIterator(); It; ++It)
	{
		UPrimitiveComponent* Primitive = It.Key().Get();
		if (Primitive && Covered.Contains(Primitive))
		{
			continue;
		}

		if (Primitive)
		{
			Primitive->SetRenderCustomDepth(It.Value().bRenderCustomDepth);
			Primitive->SetCustomDepthStencilValue(It.Value().Stencil);
		}
		It.RemoveCurrent();
	}

	for (UPrimitiveComponent* Primitive : Covered)
	{
		if (!Originals.Contains(Primitive))
		{
			FOriginal Original;
			Original.bRenderCustomDepth = Primitive->bRenderCustomDepth;
			Original.Stencil = Primitive->CustomDepthStencilValue;
			Originals.Add(Primitive, Original);
		}

		const FRequest* Top = FindTopRequest(Primitive);
		Primitive->SetRenderCustomDepth(true);
		Primitive->SetCustomDepthStencilValue(Top->Stencil);
	}
}
