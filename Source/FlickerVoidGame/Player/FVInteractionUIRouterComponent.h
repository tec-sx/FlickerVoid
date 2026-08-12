#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Interaction/FVUIInteractionMessageTypes.h"
#include "FVInteractionUIRouterComponent.generated.h"

struct FFVInteractionOfferChangedMessage;

/**
 * Translates the gameplay-side interaction offer into pure presentation data
 * and rebroadcasts it on the FVUI channel. The UI never sees FVGameplay types.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FLICKERVOID_API UFVInteractionUIRouterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVInteractionUIRouterComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Designer-configured key hints, keyed by input slot rather than action tag. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Input Hints")
	TMap<EFVInteractionSlot, FText> SlotInputHints;

private:
	void HandleOfferChanged(FGameplayTag Channel, const FFVInteractionOfferChangedMessage& Message);

	static bool ArePromptsEquivalent(
		const FFVUIInteractionPromptMessage& A,
		const FFVUIInteractionPromptMessage& B);

	FGameplayMessageListenerHandle ListenerHandle;

	/** Last broadcast payload, used to suppress redundant UI updates. */
	FFVUIInteractionPromptMessage LastPrompt;
	bool bHasBroadcast = false;
};
