#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "FVPlayerController.generated.h"

class UFVAbilitySystemComponent;
class AFVPlayerCharacter;
class UAbilitySystemComponent;
class AFVPlayerState;
class UFVInputConfig;
class UInputMappingContext;
class UFVInventoryUIRouterComponent;
class UFVDialogueUIRouterComponent;
class UFVInteractionUIRouterComponent;
class UFVInteractionDebugComponent;
struct FInputActionValue;

UCLASS(Config = Game)
class FLICKERVOID_API AFVPlayerController : public APlayerController, public IAbilitySystemInterface
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
	AFVPlayerCharacter* GetControlledCharacter() const { return CachedCharacter.Get(); }
	
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerState")
	UFVAbilitySystemComponent* GetFVAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
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
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVInventoryUIRouterComponent> InventoryUIRouterComponent;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVDialogueUIRouterComponent> DialogueUIRouterComponent;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVInteractionUIRouterComponent> InteractionUIRouterComponent;

	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVInteractionDebugComponent> InteractionDebugComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "FlickerVoid|PlayerController")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;

	TWeakObjectPtr<AFVPlayerCharacter> CachedCharacter;

    TArray<uint32> AbilityBindHandles;
    
    bool bInputMappingsAdded = false;
};
