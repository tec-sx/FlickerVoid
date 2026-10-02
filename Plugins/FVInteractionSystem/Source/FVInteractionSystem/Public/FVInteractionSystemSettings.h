#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/FVInteractionTypes.h"
#include "Core/FVInteractionGameplayTags.h"
#include "FVInteractionSystemSettings.generated.h"

class UFVInteractionUISettings;

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionHighlightSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup")
	EFVHighlightType HighlightType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EFVHighlightType::PostProcessing"))
	int32 StencilID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Highlight Setup", meta=(EditCondition="HighlightType==EFVHighlightType::OverlayMaterial"))
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	FFVInteractionHighlightSetup()
		: HighlightType(EFVHighlightType::OverlayMaterial)
		, StencilID(133)
		, HighlightMaterial(nullptr)
	{ }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	EFVInteractorState DefaultInteractorState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InteractorSettings")
	FGameplayTag InteractorTag;

	FFVInteractorSettings()
		: DefaultInteractorState(EFVInteractorState::Idle)
		, CollisionChannel(ECC_Visibility)
		, InteractorTag(FVInteractionGameplayTags::Interactor_Tag_Player)
	{ }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractableSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, Units="s", NoResetToDefault))
	float DefaultInteractionPeriod;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	EFVInteractableState DefaultInteractableState;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	TEnumAsByte<ECollisionChannel> DefaultCollisionChannel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	uint8 DefaultInteractionHighlight : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	FFVInteractionHighlightSetup DefaultHighlightSetup;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, NoResetToDefault))
	int32 DefaultInteractableWeight;

	FFVInteractableSettings()
		: DefaultInteractionPeriod(3.f)
		, DefaultInteractableState(EFVInteractableState::Idle)
		, DefaultCollisionChannel(ECC_Camera)
		, DefaultInteractionHighlight(true)
		, DefaultHighlightSetup(FFVInteractionHighlightSetup())
		, DefaultInteractableWeight(1)
	{ }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractionRegistrySettings
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractionRegistrySettings", meta=(ClampMin="0", Units="cm"))
	float DefaultActivationRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractionRegistrySettings", meta=(UIMin=0.05, Units="s"))
	float RefreshInterval;

	FFVInteractionRegistrySettings()
		: DefaultActivationRadius(600.f)
		, RefreshInterval(0.6f)
	{ }
};

UCLASS(Config = Game, DefaultConfig, NotBlueprintable, meta = (DisplayName = "Interaction System"))
class FVINTERACTIONSYSTEM_API UFVInteractionSystemSettings final : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer);

	static const UFVInteractionSystemSettings& Get() { return *GetDefault<UFVInteractionSystemSettings>(); }

	UPROPERTY(Config, BlueprintReadOnly, EditAnywhere, Category = "Interaction")
	FFVInteractorSettings InteractorDefaultSettings;

	UPROPERTY(Config, BlueprintReadOnly, EditAnywhere, Category = "Interaction")
	FFVInteractableSettings InteractableBaseSettings;
	
	UPROPERTY(Config, BlueprintReadOnly, EditAnywhere, Category = "registry")
	FFVInteractionRegistrySettings RegistrySettings;
	
	UPROPERTY(Config, BlueprintReadOnly, EditAnywhere, Category = "Widgets", meta=(Units="s", UIMin=0.001, ClampMin=0.001))
	float WidgetUpdateFrequency;

	UPROPERTY(Config, BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSoftObjectPtr<UFVInteractionUISettings> InteractionUISettings;

	UFVInteractionSystemSettings()
		: WidgetUpdateFrequency(0.05f)
	{ }
};
