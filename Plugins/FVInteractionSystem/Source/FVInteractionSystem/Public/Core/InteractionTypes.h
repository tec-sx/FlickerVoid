#pragma once
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"

#include "InteractionTypes.generated.h"

class IInteractableInterface;
class UTexture2D;

class UInteractableComponent;
class UInteractorComponent;
class UInteractionRequirement;
//
// UENUM(BlueprintType)
// enum class EInteractableLifecycle : uint8
// {
// 	OneShot		UMETA(DisplayName = "OneShot", Tooltip="Once interacted, interaction is disabled."),
// 	Repeatable	UMETA(DisplayName = "Repeatable", Tooltip="Interaction can be repeaded n times."),
// 	Default		UMETA(Hidden)
// };
//
// UENUM(BlueprintType)
// enum class EInteractorPrecision : uint8
// {
// 	High	UMETA(DisplayName = "High Precision", Tooltip = "Using Line Tracing to find Interactables."),
// 	Low		UMETA(DisplayName = "Low Precision", Tooltip = "Using Box Overlap to find Interactables."),
// 	Default	UMETA(Hidden)
// };
//
// UENUM(BlueprintType, meta=(ScriptName="InteractorState"))
// enum class EInteractorState : uint8
// {
// 	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. No Interactables in range."),
// 	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactor is looking for Interactables."),
// 	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
// 	Interacting	UMETA(DisplayName = "Interactiong", ToolTip = "Interactor is in use."),
// 	Default		UMETA(Hidden)
// };
//
//
// UENUM(BlueprintType, meta=(ScriptName="InteractableState"))
// enum class EInteractableState : uint8
// {
// 	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. Interactable is not in player range."),
// 	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactable can react to Interactor."),
// 	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
// 	Interacting	UMETA(DisplayName = "Interactiong", ToolTip = "Interactable is in use."),
// 	Paused		UMETA(DisplayName = "Paused", ToolTip = "Interaction is paused, waiting for player input."),
// 	Cooldown	UMETA(DisplayName = "Cooldown", ToolTip = "Interactions are disabled during cooldown period"),
// 	Completed	UMETA(DisplayName = "Completed", ToolTip = "Interaction is disabled, Cannot be activated again."),
// 	Default		UMETA(Hidden)
// };
//
// UENUM(BlueprintType, meta=(ScriptName="HighlightType"))
// enum class EHighlightType : uint8
// {
// 	PostProcessing	UMETA(DisplayName="Post Processing", Tooltip="Highly optimised, requires Project setup."),
// 	OverlayMaterial	UMETA(DisplayName="Overlay Material", Tooltip="For very complex meshes might cause performance issues."),
// 	Default			UMETA(Hidden)
// };
//
// UENUM(BlueprintType, meta=(ScriptName="HighlightSetupType"))
// enum class EHighlightSetupType : uint8
// {
// 	FullAll		UMETA(DisplayName="Full Auto Setup", Tooltip="Add all components from Owning Actor to Highlightable and Collision Components."),
// 	AllParent	UMETA(DisplayName="All Parents Auto Setup", Tooltip="Add all parent components to Highlightable and Collision Components."),
// 	Quick		UMETA(DisplayName="Quick Auto Setup", Tooltip="Add only first parent component to Highlightable and Collision Components."),
// 	None		UMETA(DisplayName="None",Tooltip="No auto setup will be performed."),
// 	Default		UMETA(Hidden)
// };
//
// USTRUCT(BlueprintType)
// struct FInteractionHighlightSetup
// {
// 	GENERATED_BODY()
// 	
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup")
// 	EHighlightType HighlightType;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EHighlightType::PostProcessing"))
// 	int32 StencilID;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="Highlight Setup", meta=(EditCondition="HighlightType==EHighlightType::OverlayMaterial"))
// 	TObjectPtr<UMaterialInterface> HighlightMaterial;
//
// 	FInteractionHighlightSetup()
// 	{
// 		HighlightType = EHighlightType::OverlayMaterial;
// 		StencilID = 133;
// 		HighlightMaterial = nullptr;
// 	}
// };

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Interactor;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Target;

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

	UPROPERTY(EditAnywhere, Instanced, Category = "Interaction")
	TArray<TObjectPtr<UInteractionRequirement>> Requirements;

	bool IsValid() const { return InputTag.IsValid() && ActionTag.IsValid(); }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractionKeyBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Glyph;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteraction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ActionTag;

	UPROPERTY(BlueprintReadOnly)
	bool bCanExecute = false;

	bool CanExecute() const { return bCanExecute; }

	bool operator==(const FInteraction& Other) const
	{
		return InputTag == Other.InputTag && ActionTag == Other.ActionTag && bCanExecute == Other.bCanExecute;
	}

	bool operator!=(const FInteraction& Other) const { return !(*this == Other); }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionFocusChanged, UInteractableComponent*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionOffersChanged, const TArray<FInteraction>&, Prompts);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionExecuted, const FGameplayTag&, ActionTag, UInteractorComponent*,  Interactor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableSelected,	const TScriptInterface<IInteractableInterface>&, SelectedInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableFound, const TScriptInterface<IInteractableInterface>&, FoundInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableLost, const TScriptInterface<IInteractableInterface>&, LostInteractable);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionKeyPressed,	const float&, TimeKeyPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionKeyReleased, const float&, TimeKeyReleased);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIgnoredActorAdded,	const AActor*, AddedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIgnoredActorRemoved, const AActor*, RemovedActor);

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStateChanged, const EInteractorState&, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCollisionChanged, const TEnumAsByte<ECollisionChannel>&, NewCollisionChannel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractorTagChanged, const FGameplayTag&, NewTag);