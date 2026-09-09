#pragma once

#include "CoreMinimal.h"
#include "FVInteractionSystemSettings.h"
#include "UObject/Interface.h"
#include "Core/InteractionTypes.h"
#include "InteractorInterface.generated.h"

class IInteractableInterface;

UINTERFACE()
class UInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

class FVINTERACTIONSYSTEM_API IInteractorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void Initialize(FInteractorSettings Settings) const;
	virtual bool HasInteractable_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	bool CanInteract() const;
	virtual bool CanInteract_Implementation() const = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void InteractableFound(const TScriptInterface<IInteractableInterface>& FoundInteractable);
	virtual void InteractableFound_Implementation(const TScriptInterface<IInteractableInterface>& FoundInteractable) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void InteractableLost(const TScriptInterface<IInteractableInterface>& LostInteractable);
	virtual void InteractableLost_Implementation(const TScriptInterface<IInteractableInterface>& LostInteractable) = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void AddIgnoredActors(const TArray<AActor*>& IgnoredActors);
	virtual void AddIgnoredActors_Implementation(const TArray<AActor*>& IgnoredActors) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void RemoveIgnoredActor(AActor* UnignoredActor);
	virtual void RemoveIgnoredActor_Implementation(AActor* UnignoredActor) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void RemoveIgnoredActors(const TArray<AActor*>& UnignoredActors);
	virtual void RemoveIgnoredActors_Implementation(const TArray<AActor*>& UnignoredActors) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	TArray<AActor*> GetIgnoredActors() const;
	virtual TArray<AActor*> GetIgnoredActors_Implementation() const = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	ECollisionChannel GetResponseChannel() const;
	virtual ECollisionChannel GetResponseChannel_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void SetResponseChannel(const ECollisionChannel NewResponseChannel);
	virtual void SetResponseChannel_Implementation(const ECollisionChannel NewResponseChannel) = 0;
	
	
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	// EInteractorState GetState() const;
	// virtual EInteractorState GetState_Implementation() const = 0;
	//
	// UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	// void SetState(const EInteractorState NewState);
	// virtual void SetState_Implementation(const EInteractorState NewState) = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	void SetActiveInteractable(const TScriptInterface<IInteractableInterface>& NewInteractable);
	virtual void SetActiveInteractable_Implementation(const TScriptInterface<IInteractableInterface>& NewInteractable) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	TScriptInterface<IInteractableInterface> GetActiveInteractable() const;
	virtual TScriptInterface<IInteractableInterface> GetActiveInteractable_Implementation() const = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	AActor* GetOwningActor() const;
	virtual AActor* GetOwningActor_Implementation() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FlickerVoid|Interaction|Interactor")
	bool HasFocusedInteractable() const;
	virtual bool HasFocusedInteractable_Implementation() const = 0;
};
