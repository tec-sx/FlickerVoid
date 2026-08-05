// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVDialogueMessageTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVDialogueMessageLibrary.generated.h"

struct FGameplayMessageListenerHandle;

UCLASS()
class FLICKERVOIDNARRATIVE_API UFVDialogueMessageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	//-----------------------------------------------------------------
	// Broadcast helpers (call from Flow node / Director)
	//-----------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastLineReady(const UObject* WorldContextObject, const FFVDialogueLineMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastChoicesReady(const UObject* WorldContextObject, const FFVDialogueChoicesMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastEvent(const UObject* WorldContextObject, const FFVDialogueEventMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastDialogueEnded(const UObject* WorldContextObject, const FFVDialogueEndedMessage& Message);
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastDialogueSubmitChoice(const UObject* WorldContextObject, const FFVDialogueSubmitChoiceMessage& Message);
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Messages", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastDialogueContinue(const UObject* WorldContextObject, const FFVDialogueEndedMessage& Message);
};
