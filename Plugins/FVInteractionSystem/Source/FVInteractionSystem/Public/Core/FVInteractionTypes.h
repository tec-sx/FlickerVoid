#pragma once
#include "GameplayTagContainer.h"
#include "Core/FVInputTypes.h"

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

UENUM(BlueprintType, meta=(ScriptName="HighlightSetupType"))
enum class EFVHighlightSetupType : uint8
{
	FullAll		UMETA(DisplayName="Full Auto Setup", Tooltip="Add all components from Owning Actor to Highlightable and Collision Components."),
	AllParent	UMETA(DisplayName="All Parents Auto Setup", Tooltip="Add all parent components to Highlightable and Collision Components."),
	Quick		UMETA(DisplayName="Quick Auto Setup", Tooltip="Add only first parent component to Highlightable and Collision Components."),
	None		UMETA(DisplayName="None", Tooltip="No auto setup will be performed."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractionGate"))
enum class EFVInteractionGate : uint8
{
	Disable	UMETA(DisplayName="Disable", Tooltip="Offer stays visible but cannot be executed."),
	Hide	UMETA(DisplayName="Hide", Tooltip="Offer is not shown at all while unmet.")
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FFVGesture Gesture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements", meta = (Tooltip = "Require every tag in RequiredTags instead of any one of them."))
	bool bRequireAllTags = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	EFVInteractionGate RequirementGate = EFVInteractionGate::Disable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 Weight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1", Tooltip = "-1 is unlimited. 0 means the action is exhausted."))
	int32 RemainingUses = -1;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (Tooltip = "Evaluated per interactor when offers are refreshed."))
	bool bRequirementsMet = false;

	bool IsValid() const { return InputTag.IsValid() && ActionTag.IsValid(); }
	bool IsExhausted() const { return RemainingUses == 0; }
	bool CanExecute() const { return bRequirementsMet && RemainingUses != 0; }

	bool AreTagsSatisfied(const FGameplayTagContainer& SourceTags) const
	{
		if (SourceTags.HasAny(BlockedTags))
		{
			return false;
		}

		if (RequiredTags.IsEmpty())
		{
			return true;
		}

		return bRequireAllTags ? SourceTags.HasAll(RequiredTags) : SourceTags.HasAny(RequiredTags);
	}

	bool operator==(const FFVInteractionOffer& Other) const
	{
		return InputTag == Other.InputTag &&
			ActionTag == Other.ActionTag &&
			Gesture == Other.Gesture &&
			bRequirementsMet == Other.bRequirementsMet &&
			RequirementGate == Other.RequirementGate &&
			RemainingUses == Other.RemainingUses &&
			Weight == Other.Weight;
	}

	bool operator!=(const FFVInteractionOffer& Other) const { return !(*this == Other); }
};