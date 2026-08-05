// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FVCallOutTableRow.generated.h"


USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVCallOutTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> Voice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DisplayDuration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bShowTalkIcon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTag IdentityTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTagContainer RequiredTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTagContainer BlockingTags;
};
