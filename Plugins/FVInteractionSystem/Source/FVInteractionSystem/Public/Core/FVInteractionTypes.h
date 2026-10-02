#pragma once
#include "GameplayTagContainer.h"
#include "Input/Core/FVInputTypes.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDisplayInfo.h"

#include "FVInteractionTypes.generated.h"

class UFVInteractableComponent;
class UFVInteractorComponent;
class UMaterialInterface;

UENUM(BlueprintType, meta=(ScriptName="OcclusionDetectionMode"))
enum class EFVOcclusionDetectionMode : uint8
{
	None		UMETA(DisplayName="None", Tooltip="No occlusion validation is performed."),
	Location	UMETA(DisplayName="Location", Tooltip="Validate against the interactable's focus point."),
	Socket		UMETA(DisplayName="Socket", Tooltip="Validate against a named socket on the target mesh."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractorState"))
enum class EFVInteractorState : uint8
{
	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. No Interactables in range."),
	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactor is looking for Interactables."),
	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
	Interacting	UMETA(DisplayName = "Interacting", ToolTip = "Interactor is in use."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractableState"))
enum class EFVInteractableState : uint8
{
	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. Interactable is not in player range."),
	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactable can react to Interactor."),
	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
	Interacting	UMETA(DisplayName = "Interacting", ToolTip = "Interactable is in use."),
	Paused		UMETA(DisplayName = "Paused", ToolTip = "Interaction is paused, waiting for player input."),
	Cooldown	UMETA(DisplayName = "Cooldown", ToolTip = "Interactions are disabled during cooldown period"),
	Completed	UMETA(DisplayName = "Completed", ToolTip = "Interaction is disabled, Cannot be activated again."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="HighlightType"))
enum class EFVHighlightType : uint8
{
	PostProcessing	UMETA(DisplayName="Post Processing", Tooltip="Highly optimised, requires Project setup."),
	OverlayMaterial	UMETA(DisplayName="Overlay Material", Tooltip="For very complex meshes might cause performance issues."),
	Default			UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionCommit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFVInteractorComponent> Interactor;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UFVInteractableComponent> Interactable;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ShowOnlyInnerProperties))
	FFVGesture Gesture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Display")
	FFVDisplayInfo Display;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	FFVConditionSet Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Effects")
	FFVEffectList Effects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0"))
	int32 Weight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "-1", Tooltip = "-1 is unlimited. 0 means the action is exhausted."))
	int32 RemainingUses = -1;

	bool IsValid() const { return InputTag.IsValid() && ActionTag.IsValid(); }
	bool IsExhausted() const { return RemainingUses == 0; }
	bool HidesWhenUnavailable() const { return Conditions.FailurePresentation == EFVConditionFailurePresentation::Hidden; }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionOfferData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Interaction", meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction", meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVGesture Gesture;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FFVDisplayInfo Display;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	int32 Weight = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	int32 RemainingUses = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FText LockedReason;

	bool CanExecute() const { return bAvailable && RemainingUses != 0; }

	static FFVInteractionOfferData From(const FFVInteractionOffer& Offer, const bool bInAvailable)
	{
		FFVInteractionOfferData Data;
		Data.InputTag = Offer.InputTag;
		Data.ActionTag = Offer.ActionTag;
		Data.Gesture = Offer.Gesture;
		Data.Display = Offer.Display;
		Data.Weight = Offer.Weight;
		Data.RemainingUses = Offer.RemainingUses;
		Data.bAvailable = bInAvailable;

		if (!bInAvailable && Offer.Conditions.FailurePresentation == EFVConditionFailurePresentation::ShowLockedWithReason)
		{
			Data.LockedReason = Offer.Conditions.FailureReason.IsEmpty() ? Offer.Conditions.GetDescription() : Offer.Conditions.FailureReason;
		}
		return Data;
	}

	bool operator==(const FFVInteractionOfferData& Other) const
	{
		return InputTag == Other.InputTag &&
			ActionTag == Other.ActionTag &&
			Gesture == Other.Gesture &&
			Weight == Other.Weight &&
			RemainingUses == Other.RemainingUses &&
			bAvailable == Other.bAvailable &&
			LockedReason.EqualTo(Other.LockedReason);
	}

	bool operator!=(const FFVInteractionOfferData& Other) const { return !(*this == Other); }
};

