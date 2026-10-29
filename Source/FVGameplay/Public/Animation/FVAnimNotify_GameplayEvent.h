#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "FVAnimNotify_GameplayEvent.generated.h"

/** Sends a gameplay event to the mesh owner at this frame, so the ability playing the montage can act on it (attach a prop, apply an item, hand it over). */
UCLASS(const, HideCategories = Object, CollapseCategories, meta = (DisplayName = "FV Gameplay Event"))
class FLICKERVOIDGAMEPLAY_API UFVAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "Event", ToolTip = "Event sent to the mesh owner, with the owner as instigator, e.g. Event.Item.Notify.Apply."))
	FGameplayTag EventTag;
};
