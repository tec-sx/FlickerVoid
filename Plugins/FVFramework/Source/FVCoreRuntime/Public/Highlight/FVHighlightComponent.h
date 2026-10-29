#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVHighlightComponent.generated.h"

class UPrimitiveComponent;

/**
 * The one writer of custom depth on its actor's primitives. Highlight sources (interaction focus, scanner, quest markers)
 * push and pop requests; each primitive shows the stencil of its highest priority request and gets its own values back
 * when no request covers it.
 */
UCLASS(ClassGroup = (FlickerVoid), meta = (BlueprintSpawnableComponent))
class FVCORERUNTIME_API UFVHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVHighlightComponent();

	/** The actor's highlight component, added at runtime when the actor has none. */
	static UFVHighlightComponent* FindOrAdd(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "FV|Highlight", meta = (ToolTip = "Shows Stencil on Primitives until Source pops it. A new push from the same Source replaces its request; on equal priority the latest push shows."))
	void PushHighlight(FName Source, int32 Stencil, int32 Priority, const TArray<UPrimitiveComponent*>& Primitives);

	UFUNCTION(BlueprintCallable, Category = "FV|Highlight", meta = (ToolTip = "Removes Source's request; primitives no request covers get their own custom depth back."))
	void PopHighlight(FName Source);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FRequest
	{
		FName Source;
		int32 Stencil = 0;
		int32 Priority = 0;
		TArray<TWeakObjectPtr<UPrimitiveComponent>> Primitives;
	};

	struct FOriginal
	{
		bool bRenderCustomDepth = false;
		int32 Stencil = 0;
	};

	void Refresh();
	const FRequest* FindTopRequest(const UPrimitiveComponent* Primitive) const;

	/** In push order, so a later request wins a priority tie. */
	TArray<FRequest> Requests;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, FOriginal> Originals;
};
