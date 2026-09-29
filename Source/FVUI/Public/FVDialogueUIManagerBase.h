#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"
#include "FVDialogueUIManagerBase.generated.h"

struct FFVUIDialogueEndedMessage;
struct FFVUIDialogueEventMessage;
struct FFVUIDialogueChoicesMessage;
struct FFVUIDialogueLineMessage;

UCLASS(BlueprintType, Blueprintable)
class FLICKERVOIDUI_API UFVDialogueUIManagerBase : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue", meta = (WorldContext = "WorldContextObject"))
	static UFVDialogueUIManagerBase* Get(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Initialize(TSubclassOf<UUserWidget> DialogueWidgetClass, APlayerController* InPC);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Deinitialize();
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SubmitChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SubmitContinue();
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnManagerInitialized(TSubclassOf<UUserWidget> DialogueWidgetClass, APlayerController* InPC);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnManagerDeinitialized();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueLineReady(const FFVUIDialogueLineMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueChoicesReady(const FFVUIDialogueChoicesMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEvent(const FFVUIDialogueEventMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEnded(const FFVUIDialogueEndedMessage& Message);

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPC;

private:
	void RegisterListeners();
	void UnregisterListeners();

	void HandleLineReady(FGameplayTag Channel, const FFVUIDialogueLineMessage& Message);
	void HandleChoicesReady(FGameplayTag Channel, const FFVUIDialogueChoicesMessage& Message);
	void HandleEvent(FGameplayTag Channel, const FFVUIDialogueEventMessage& Message);
	void HandleEnded(FGameplayTag Channel, const FFVUIDialogueEndedMessage& Message);

	FGameplayMessageListenerHandle LineReadyHandle;
	FGameplayMessageListenerHandle ChoicesReadyHandle;
	FGameplayMessageListenerHandle EventHandle;
	FGameplayMessageListenerHandle EndedHandle;

	bool bIsInitialized = false;
};
