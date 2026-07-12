#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "FVTagComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLICKERVOIDCORE_API UFVTagComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	FGameplayTagContainer& GetOwnedTags() { return  OwnedTags; }
	
	UFUNCTION(BlueprintPure)
	void AddTag(FGameplayTag& InTag) { OwnedTags.AddTag(InTag); }
	
	UFUNCTION(BlueprintPure)
	void RemoveTag(FGameplayTag& InTag) { OwnedTags.RemoveTag(InTag); }
	
	UFUNCTION(BlueprintPure)
	bool HasTag(FGameplayTag& InTag) const { return OwnedTags.HasTag(InTag); }
	
	UFUNCTION(BlueprintPure)
	bool HasAnyTagExact(FGameplayTagContainer& InTags) const { return OwnedTags.HasAllExact(InTags); }
	
	UFUNCTION(BlueprintPure)
	bool HasAllTagsExact(FGameplayTagContainer& InTags) const { return OwnedTags.HasAnyExact(InTags); }

private:
	FGameplayTagContainer OwnedTags;
};
