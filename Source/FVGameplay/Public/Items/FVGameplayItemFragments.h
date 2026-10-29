#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Items/FVItemDefinition.h"
#include "FVGameplayItemFragments.generated.h"

class AActor;
class UAnimMontage;
class UFVInteractableDefinition;
class USoundBase;
class UStaticMesh;

/** How the item shows in the world and in a hand. */
USTRUCT(BlueprintType, meta = (DisplayName = "World"))
struct FLICKERVOIDGAMEPLAY_API FFVItemFragment_World : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ToolTip = "Mesh of the item lying in the world, held or used."))
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ToolTip = "Spawned instead of the generic world item when dropped or held (e.g. a flashlight with a light)."))
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World", meta = (ToolTip = "Overrides the offers chosen by category."))
	TObjectPtr<UFVInteractableDefinition> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World|Hand", meta = (ToolTip = "Socket on the item mesh placed in the hand; none = mesh origin."))
	FName GripSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World|Hand", meta = (ToolTip = "Offset from the hand socket while held or used."))
	FTransform GripOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World|Sound", meta = (ToolTip = "Played when the item is picked up."))
	TSoftObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World|Sound", meta = (ToolTip = "Played when the item is dropped."))
	TSoftObjectPtr<USoundBase> DropSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World|Sound", meta = (ToolTip = "Played when the item takes effect."))
	TSoftObjectPtr<USoundBase> UseSound;
};

/** Animation styles the character's item animations map to montages and poses. */
USTRUCT(BlueprintType, meta = (DisplayName = "Animation"))
struct FLICKERVOIDGAMEPLAY_API FFVItemFragment_Animation : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (Categories = "Animation.Item.Use", ToolTip = "How the item is used, e.g. Animation.Item.Use.Drink. Empty = Animation.Item.Use."))
	FGameplayTag UseStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (Categories = "Animation.Item.Hold", ToolTip = "How the item is held in a hand. Empty = Animation.Item.Hold."))
	FGameplayTag HoldStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (ToolTip = "Overrides the montage of the use style."))
	TSoftObjectPtr<UAnimMontage> UseMontage;
};

/** Can be thrown to make noise. */
USTRUCT(BlueprintType, meta = (DisplayName = "Throwable"))
struct FLICKERVOIDGAMEPLAY_API FFVItemFragment_Throwable : public FFVItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = 0, ToolTip = "Loudness of the impact noise heard by AI."))
	float Loudness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = 0, Units = "cm", ToolTip = "How far the impact can be heard; 0 = the listener's own hearing range."))
	float NoiseRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable", meta = (ClampMin = 0, Units = "cm/s", ToolTip = "Speed the item leaves the hand with."))
	float ThrowSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable", meta = (ToolTip = "The item is destroyed on its first impact, e.g. a bottle."))
	bool bBreakOnImpact = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable", meta = (ToolTip = "Played on the first impact."))
	TSoftObjectPtr<USoundBase> ImpactSound;
};
