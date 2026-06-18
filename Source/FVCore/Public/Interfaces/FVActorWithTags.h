// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "FVActorWithTags.generated.h"

UINTERFACE()
class UFVActorWithTags : public UInterface
{
	GENERATED_BODY()
};

class FLICKERVOIDCORE_API IFVActorWithTags
{
	GENERATED_BODY()

public:
	virtual FGameplayTagContainer& GetAllTags() = 0;
	virtual void AddTag(FGameplayTag& Tag) = 0;
	virtual void RemoveTag(FGameplayTag& Tag) = 0;
	virtual bool HasAnyTagExact(FGameplayTagContainer& Tags) const = 0;
	virtual bool HasAllTagsExact(FGameplayTagContainer& Tags) const = 0;
	virtual bool HasTag(FGameplayTag& Tag) const = 0;
};
