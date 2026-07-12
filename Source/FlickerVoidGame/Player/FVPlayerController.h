#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "FVPlayerController.generated.h"

class UFVInteractionSubsystem;
class UAbilitySystemComponent;
class AFVPlayerState;
class AFVCharacter;
class UFVInputConfig;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(Config = Game)
class FLICKERVOID_API AFVPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFVPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    // ========================================================================
    // INPUT CONFIGURATION
    // ========================================================================

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UFVInputConfig> InputConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TArray<TSoftObjectPtr<UInputMappingContext>> DefaultInputMappings;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    int32 DefaultInputPriority = 0;

    // ========================================================================
    // POSSESSION
    // ========================================================================

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerController")
	AFVCharacter* GetControlledCharacter() const { return CachedCharacter; }

protected:
    // ========================================================================
    // INPUT SETUP
    // ========================================================================

    virtual void SetupInputComponent() override;

    void InitializeInput();
    void AddInputMappingContexts();
    void RemoveInputMappingContexts();

    // ========================================================================
    // INPUT CALLBACKS - Movement
    // ========================================================================

	void Input_MoveCompleted(const FInputActionValue& Value);
	void Input_MoveTriggered(const FInputActionValue& Value);
	void Input_LookTriggered(const FInputActionValue& Value);
    void Input_CrouchTriggered(const FInputActionValue& Value);
    void Input_WalkTriggered(const FInputActionValue& Value);
    void Input_SprintTriggered(const FInputActionValue& Value);
    void Input_SprintCompleted(const FInputActionValue& Value);
    void Input_JumpStarted(const FInputActionValue& Value);
    void Input_JumpTriggered(const FInputActionValue& Value);
	void Input_AimStarted(const FInputActionValue& Value);
    void Input_AimCompleted(const FInputActionValue& Value);

    // ========================================================================
    // INPUT CALLBACKS - Abilities
    // ========================================================================

    void Input_AbilityInputTagPressed(FGameplayTag InputTag);
    void Input_AbilityInputTagReleased(FGameplayTag InputTag);

private:
	UPROPERTY(Transient)
    TObjectPtr<AFVCharacter> CachedCharacter;
	
	mutable TWeakObjectPtr<UFVInteractionSubsystem> InteractionSubsystem;

    TArray<uint32> AbilityBindHandles;
    
    bool bInputMappingsAdded = false;
};
