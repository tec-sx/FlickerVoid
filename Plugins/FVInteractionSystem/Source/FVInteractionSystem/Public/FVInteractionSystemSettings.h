

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/FVInteractionTypes.h"
#include "FVInteractionSystemSettings.generated.h"

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVDetectionSetup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	TEnumAsByte<ECollisionChannel> ValidationCollisionChannel = ECC_Camera;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	EFVOcclusionDetectionMode SafetyTracingMode = EFVOcclusionDetectionMode::Socket;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	FName ActorMeshName = FName("CharacterMesh0");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	FName StartSocketName = FName("head");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup", meta=(UIMin=0.01, ClampMin=0.01, Units="s"))
	float TracingInterval = 0.05f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup", meta=(UIMin=0, ClampMin=0, Units="cm"))
	float TracingRange = 600.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup", meta=(UIMin=0, ClampMin=0, Units="cm"))
	float TracingShapeHalfSize = 15.f;

	bool operator==(const FFVDetectionSetup& Other) const
	{
		return ActorMeshName == Other.ActorMeshName && StartSocketName == Other.StartSocketName;
	}

	bool operator!=(const FFVDetectionSetup& Other) const { return !(*this == Other); }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionHighlightSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup")
	EFVHighlightType HighlightType = EFVHighlightType::OverlayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EFVHighlightType::PostProcessing"))
	int32 StencilID = 133;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EFVHighlightType::OverlayMaterial"))
	TObjectPtr<UMaterialInterface> HighlightMaterial = nullptr;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	EFVInteractorState DefaultInteractorState = EFVInteractorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	EFVInteractableDetectionMode DefaultPrecision = EFVInteractableDetectionMode::Trace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	TEnumAsByte<ECollisionChannel> InteractorCollisionChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	FFVDetectionSetup DetectionSetup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	FGameplayTag InteractorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings", meta=(UIMin=0.05, ClampMin=0.05, Units="s"))
	float BroadPhaseInterval = 0.2f;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractableSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, Units="s", NoResetToDefault))
	float DefaultInteractionPeriod = 3.f;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	EFVInteractableState DefaultInteractableState = EFVInteractableState::Idle;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	TEnumAsByte<ECollisionChannel> DefaultCollisionChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, Category="InteractableSettings")
	EFVHighlightSetupType DefaultHighlightSetupType = EFVHighlightSetupType::Quick;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	uint8 DefaultInteractionHighlight : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	FGameplayTag InteractableMainTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	FFVInteractionHighlightSetup DefaultHighlightSetup;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault, UIMin=0.1, ClampMin=0.1, Units="s"))
	float DefaultCooldownPeriod = 3.f;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, NoResetToDefault))
	int32 DefaultInteractableWeight = 1;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=0, ClampMin=0, Units="cm"))
	float DefaultDetectionRadius = 150.f;

	FFVInteractableSettings()
		: DefaultInteractionHighlight(true)
	{
	}
};

UCLASS(Config = Game, DefaultConfig, NotBlueprintable, meta = (DisplayName = "Interaction System"))
class FVINTERACTIONSYSTEM_API UFVInteractionSystemSettings final : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer);

	static const UFVInteractionSystemSettings& Get() { return *GetDefault<UFVInteractionSystemSettings>(); }

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interactor")
	FFVInteractorSettings InteractorDefaultSettings;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interactable")
	FFVInteractableSettings InteractableBaseSettings;
	
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Widgets", meta=(Units="s", UIMin=0.001, ClampMin=0.001))
	float WidgetUpdateFrequency = 0.05f;
};
