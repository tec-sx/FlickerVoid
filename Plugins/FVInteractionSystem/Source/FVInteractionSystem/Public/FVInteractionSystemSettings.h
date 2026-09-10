

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/InteractionTypes.h"
#include "FVInteractionSystemSettings.generated.h"

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FTracingSetup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	TEnumAsByte<ECollisionChannel> ValidationCollisionChannel = ECC_Camera;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	ESafetyTracingMode SafetyTracingMode = ESafetyTracingMode::Socket;

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

	bool operator==(const FTracingSetup& Other) const
	{
		return ActorMeshName == Other.ActorMeshName && StartSocketName == Other.StartSocketName;
	}

	bool operator!=(const FTracingSetup& Other) const { return !(*this == Other); }
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	EInteractorState DefaultInteractorState = EInteractorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	EInteractorPrecision DefaultPrecision = EInteractorPrecision::Trace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	TEnumAsByte<ECollisionChannel> InteractorCollisionChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	FTracingSetup TracingSetup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings")
	FGameplayTag InteractorTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractorSettings", meta=(UIMin=0.05, ClampMin=0.05, Units="s"))
	float BroadPhaseInterval = 0.2f;
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FInteractableSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, Units="s", NoResetToDefault))
	float DefaultInteractionPeriod = 3.f;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	EInteractableState DefaultInteractableState = EInteractableState::Idle;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	TEnumAsByte<ECollisionChannel> DefaultCollisionChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, Category="InteractableSettings")
	EHighlightSetupType DefaultHighlightSetupType = EHighlightSetupType::Quick;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	uint8 DefaultInteractionHighlight : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	FGameplayTag InteractableMainTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InteractableSettings", meta=(NoResetToDefault))
	FInteractionHighlightSetup DefaultHighlightSetup;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault, UIMin=0.1, ClampMin=0.1, Units="s"))
	float DefaultCooldownPeriod = 3.f;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, NoResetToDefault))
	int32 DefaultInteractableWeight = 1;

	UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=0, ClampMin=0, Units="cm"))
	float DefaultDetectionRadius = 150.f;

	FInteractableSettings()
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
	FInteractorSettings InteractorDefaultSettings;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interactable")
	FInteractableSettings InteractableBaseSettings;
	
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Widgets", meta=(Units="s", UIMin=0.001, ClampMin=0.001))
	float WidgetUpdateFrequency = 0.05f;
};
