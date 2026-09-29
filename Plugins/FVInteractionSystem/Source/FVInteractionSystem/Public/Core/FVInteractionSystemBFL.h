// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVInteractionSystemBFL.generated.h"


UCLASS()
class FVINTERACTIONSYSTEM_API UFVInteractionSystemBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="FVInteractionSystem|Helpers")
	static UMeshComponent* FindMeshByTag(const FName Tag, const AActor* Actor);
};
