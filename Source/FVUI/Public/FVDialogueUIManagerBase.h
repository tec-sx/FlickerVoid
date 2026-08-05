#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"
#include "FVDialogueUIManagerBase.generated.h"

struct FFVDialogueEndedMessage;
struct FFVDialogueEventMessage;
struct FFVDialogueChoicesMessage;
struct FFVDialogueLineMessage;

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
	void OnDialogueLineReady(const FFVDialogueLineMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueChoicesReady(const FFVDialogueChoicesMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEvent(const FFVDialogueEventMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEnded(const FFVDialogueEndedMessage& Message);

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPC;

private:
	void RegisterListeners();
	void UnregisterListeners();

	void HandleLineReady(FGameplayTag Channel, const FFVDialogueLineMessage& Message);
	void HandleChoicesReady(FGameplayTag Channel, const FFVDialogueChoicesMessage& Message);
	void HandleEvent(FGameplayTag Channel, const FFVDialogueEventMessage& Message);
	void HandleEnded(FGameplayTag Channel, const FFVDialogueEndedMessage& Message);

	FGameplayMessageListenerHandle LineReadyHandle;
	FGameplayMessageListenerHandle ChoicesReadyHandle;
	FGameplayMessageListenerHandle EventHandle;
	FGameplayMessageListenerHandle EndedHandle;

	bool bIsInitialized = false;
};
