#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVDialogueTypes.h"
#include "FVDialogueParticipantComponent.generated.h"

class UFlowAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnParticipantLine, const FFVDialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFVOnParticipantBark, const FText&, Text, float, Duration);

/**
 * Makes an actor talkable and lets it react to its own lines (animation, lip sync, look-at, floating text).
 * The dialogue subsystem broadcasts these events on the speaking actor's component.
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FVDIALOGUESYSTEM_API UFVDialogueParticipantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVDialogueParticipantComponent();

	static UFVDialogueParticipantComponent* Find(const AActor* Actor);

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	bool HasDialogue() const { return Dialogue != nullptr; }

	UFUNCTION(BlueprintPure, Category = "FV|Dialogue")
	UFlowAsset* GetDialogue() const { return Dialogue; }

	/** Swaps the conversation, e.g. from a quest step. */
	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	void SetDialogue(UFlowAsset* NewDialogue) { Dialogue = NewDialogue; }

	/** Starts this actor's dialogue with Instigator. */
	UFUNCTION(BlueprintCallable, Category = "FV|Dialogue")
	bool StartDialogue(AActor* Instigator);

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnParticipantLine OnLineStarted;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnParticipantLine OnLineFinished;

	UPROPERTY(BlueprintAssignable, Category = "FV|Dialogue")
	FFVOnParticipantBark OnBark;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UFlowAsset> Dialogue;
};
