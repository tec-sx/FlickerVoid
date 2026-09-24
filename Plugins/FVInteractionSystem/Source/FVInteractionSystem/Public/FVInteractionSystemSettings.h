

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/FVInteractionTypes.h"
#include "FVInteractionSystemSettings.generated.h"

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
	EFVInteractableDetectionMode DefaultDetectionMode = EFVInteractableDetectionMode::Trace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	TEnumAsByte<ECollisionChannel> InteractorCollisionChannel = ECC_Visibility;
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

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, NoResetToDefault))
	int32 DefaultInteractableWeight = 1;

	FFVInteractableSettings()
		: DefaultInteractionHighlight(true)
	{ }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionRegistrySettings
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractionRegistrySettings", meta=(ClampMin="0", Units="cm"))
	float DefaultActivationRadius = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractionRegistrySettings", meta=(UIMin=0.05, Units="s"))
	float RefreshInterval = 0.6f;
};

UCLASS(Config = Game, DefaultConfig, NotBlueprintable, meta = (DisplayName = "Interaction System"))
class FVINTERACTIONSYSTEM_API UFVInteractionSystemSettings final : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer);

	static const UFVInteractionSystemSettings& Get() { return *GetDefault<UFVInteractionSystemSettings>(); }

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interaction")
	FFVInteractorSettings InteractorDefaultSettings;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interaction")
	FFVInteractableSettings InteractableBaseSettings;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "registry")
	FFVInteractionRegistrySettings RegistrySettings;
	
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Widgets", meta=(Units="s", UIMin=0.001, ClampMin=0.001))
	float WidgetUpdateFrequency = 0.05f;
};
