

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/InteractionTypes.h"
#include "FVInteractionSystemSettings.generated.h"

USTRUCT(BlueprintType)
struct FTracingSetup
{
	GENERATED_BODY()
	//
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	// TEnumAsByte<ECollisionChannel> ValidationCollisionChannel;
	//
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	// FName ActorMeshName;
	//
	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Setup")
	// FName StartSocketName;
	//
	// bool operator==(const FTracingSetup& Other) const
	// {
	// 	return ActorMeshName == Other.ActorMeshName && StartSocketName == Other.StartSocketName;
	// }
};

USTRUCT(BlueprintType)
struct FInteractorSettings
{
	GENERATED_BODY()

	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractorSettings")
	// EInteractorState DefaultInteractorState;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractorSettings")
	// TEnumAsByte<ECollisionChannel> InteractorCollisionChannel;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractorSettings")
	// FTracingSetup TracingSetup;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractorSettings")
	// FGameplayTag InteractorTag;
	//
	// FInteractorSettings()
	// 	: DefaultInteractorState(EInteractorState::Idle)
	// 	, InteractorCollisionChannel(ECC_Visibility)
	// {
	// 	TracingSetup.StartSocketName = FName("head");
	// 	TracingSetup.ActorMeshName = FName("CharacterMesh0");
	// 	TracingSetup.ValidationCollisionChannel = ECC_Camera;
	// }
};

USTRUCT(BlueprintType)
struct FInteractableSettings
{
	GENERATED_BODY()
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, Units="seconds", NoResetToDefault))
	// float DefaultInteractionPeriod;
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	// EInteractableState DefaultInteractableState;
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault))
	// TEnumAsByte<ECollisionChannel> DefaultCollisionChannel;
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings")
	// EHighlightSetupType DefaultHighlightSetupType;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractableSettings", meta=(NoResetToDefault))
	// uint8 DefaultInteractionHighlight : 1;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractableSettings", meta=(NoResetToDefault))
	// FGameplayTag InteractableMainTag;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractableSettings", meta=(NoResetToDefault))
	// FInteractionHighlightSetup DefaultHighlightSetup;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractableSettings", meta=(NoResetToDefault))
	// EInteractableLifecycle DefaultLifecycleMode;
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(NoResetToDefault, EditCondition = "DefaultLifecycleMode == EInteractableLifecycle::Repeatable", UIMin=0.1, ClampMin=0.1, Units="Seconds"))
	// float DefaultCooldownPeriod;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly,  Category="InteractableSettings", meta=(NoResetToDefault, EditCondition = "DefaultLifecycleMode == EInteractableLifecycle::Repeatable"))
	// int32 DefaultLifecycles;
	//
	// UPROPERTY(EditAnywhere, Category="InteractableSettings", meta=(UIMin=-1, ClampMin=-1, NoResetToDefault))
	// int32 DefaultInteractableWeight;
	//
	// FInteractableSettings()
	// 	: DefaultInteractionPeriod(3.f)
	//   	, DefaultInteractableState(EInteractableState::Idle)
	//   	, DefaultCollisionChannel(ECC_Camera)
	//   	, DefaultHighlightSetupType(EHighlightSetupType::Quick)
	//   	, DefaultInteractionHighlight(true)
	//   	, DefaultHighlightSetup(FInteractionHighlightSetup())
	//   	, DefaultLifecycleMode(EInteractableLifecycle::Repeatable)
	//   	, DefaultCooldownPeriod(3.f)
	//   	, DefaultLifecycles(-1)
	// 	, DefaultInteractableWeight(1)
	// {
	// }
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interaction System"))
class FVINTERACTIONSYSTEM_API UFVInteractionSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFVInteractionSystemSettings(const FObjectInitializer& ObjectInitializer);
	
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interactor")
	FInteractorSettings InteractorDefaultSettings;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interactable")
	FInteractableSettings InteractableBaseSettings;
	
	UPROPERTY(config, BlueprintReadOnly, EditAnywhere, Category = "Widgets", meta=(Units="s", UIMin=0.001, ClampMin=0.001))
	float WidgetUpdateFrequency = 0.05f;
};
