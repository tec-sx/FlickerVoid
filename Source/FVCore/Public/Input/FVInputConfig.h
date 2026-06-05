#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FFVInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS(BlueprintType, Const)
class FLICKERVOIDCORE_API UFVInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Pawn")
	TArray<FGameplayTag> GetAllNativeInputTags() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|Pawn")
	TArray<FGameplayTag> GetAllAbilityInputTags() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FFVInputAction> NativeInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FFVInputAction> AbilityInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputCosmetics"))
	TObjectPtr<UDataTable> KeyIconDataTable;
};