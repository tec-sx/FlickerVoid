#pragma once
#include "GameplayTagContainer.h"

#include "InteractionTypes.generated.h"

class UInteractableComponent;
class UInteractorComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EInteractorPrecision : uint8
{
	Trace		UMETA(DisplayName = "Trace", Tooltip = "Using Line Tracing to find Interactables."),
	Overlap		UMETA(DisplayName = "Overlap", Tooltip = "Using Box Overlap to find Interactables."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractorState"))
enum class EInteractorState : uint8
{
	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. No Interactables in range."),
	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactor is looking for Interactables."),
	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
	Interacting	UMETA(DisplayName = "Interacting", ToolTip = "Interactor is in use."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractableState"))
enum class EInteractableState : uint8
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
enum class EHighlightType : uint8
{
	PostProcessing	UMETA(DisplayName="Post Processing", Tooltip="Highly optimised, requires Project setup."),
	OverlayMaterial	UMETA(DisplayName="Overlay Material", Tooltip="For very complex meshes might cause performance issues."),
	Default			UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="HighlightSetupType"))
enum class EHighlightSetupType : uint8
{
	FullAll		UMETA(DisplayName="Full Auto Setup", Tooltip="Add all components from Owning Actor to Highlightable and Collision Components."),
	AllParent	UMETA(DisplayName="All Parents Auto Setup", Tooltip="Add all parent components to Highlightable and Collision Components."),
	Quick		UMETA(DisplayName="Quick Auto Setup", Tooltip="Add only first parent component to Highlightable and Collision Components."),
	None		UMETA(DisplayName="None", Tooltip="No auto setup will be performed."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractionInputPhase"))
enum class EInteractionInputPhase : uint8
{
	Pressed		UMETA(DisplayName="Pressed", Tooltip="Input key was pressed this frame."),
	Released	UMETA(DisplayName="Released", Tooltip="Input key was released this frame."),
	Cancelled	UMETA(DisplayName="Cancelled", Tooltip="Input was aborted without a commit."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractionInputMode"))
enum class EInteractionInputMode : uint8
{
	Press		UMETA(DisplayName="Press", Tooltip="Commits immediately on press."),
	Hold		UMETA(DisplayName="Hold", Tooltip="Commits after the key is held for InteractionPeriod."),
	Mash		UMETA(DisplayName="Mash", Tooltip="Commits after RequiredPresses within InteractionPeriod."),
	Hover		UMETA(DisplayName="Hover", Tooltip="Commits after being focused for InteractionPeriod, no key needed."),
	Automatic	UMETA(DisplayName="Automatic", Tooltip="Commits as soon as the offer becomes available."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="SafetyTracingMode"))
enum class ESafetyTracingMode : uint8
{
	None		UMETA(DisplayName="None", Tooltip="No occlusion validation is performed."),
	Location	UMETA(DisplayName="Location", Tooltip="Validate against the interactable's focus point."),
	Socket		UMETA(DisplayName="Socket", Tooltip="Validate against a named socket on the target mesh."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType, meta=(ScriptName="InteractionGate"))
enum class EInteractionGate : uint8
{
	Disable	UMETA(DisplayName="Disable", Tooltip="Offer stays visible but cannot be executed."),
	Hide	UMETA(DisplayName="Hide", Tooltip="Offer is not shown at all while unmet.")
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionHighlightSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup")
	EHighlightType HighlightType = EHighlightType::OverlayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EHighlightType::PostProcessing"))
	int32 StencilID = 133;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EHighlightType::OverlayMaterial"))
	TObjectPtr<UMaterialInterface> HighlightMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionCommit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInteractorComponent> Interactor;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInteractableComponent> Interactable;

	UPROPERTY(BlueprintReadOnly)
	FVector InteractionPoint = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionOffer
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "InputTag.Interaction"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements", meta = (Tooltip = "Require every tag in RequiredTags instead of any one of them."))
	bool bRequireAllTags = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Requirements")
	EInteractionGate RequirementGate = EInteractionGate::Disable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EInteractionInputMode InputMode = EInteractionInputMode::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1", Units = "s"))
	float InteractionPeriod = -1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "InputMode==EInteractionInputMode::Mash", ClampMin = "1"))
	int32 RequiredPresses = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 Weight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1", Tooltip = "-1 is unlimited. 0 means the action is exhausted."))
	int32 RemainingUses = -1;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (Tooltip = "Evaluated per interactor when offers are refreshed."))
	bool bRequirementsMet = false;

	bool IsValid() const { return InputTag.IsValid() && ActionTag.IsValid(); }
	bool IsExhausted() const { return RemainingUses == 0; }
	bool CanExecute() const { return bRequirementsMet && !IsExhausted(); }

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

	bool operator==(const FInteractionOffer& Other) const
	{
		return InputTag == Other.InputTag &&
			ActionTag == Other.ActionTag &&
			bRequirementsMet == Other.bRequirementsMet &&
			RemainingUses == Other.RemainingUses &&
			InputMode == Other.InputMode &&
			Weight == Other.Weight;
	}

	bool operator!=(const FInteractionOffer& Other) const { return !(*this == Other); }
};