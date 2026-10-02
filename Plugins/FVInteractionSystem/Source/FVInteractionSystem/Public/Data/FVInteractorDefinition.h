#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FVInteractorDefinition.generated.h"

class UFVInteractorModeDefinition;

UCLASS(BlueprintType, Const)
class FVINTERACTIONSYSTEM_API UFVInteractorDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVInteractorDefinition();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (Categories = "Interactor.Tag"))
	FGameplayTag InteractorTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (Tooltip = "Tags this interactor initially presents to offer conditions."))
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (Tooltip = "Any offer whose ActionTag matches these is gated off entirely."))
	FGameplayTagContainer BlockedActionTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modes", meta = (Tooltip = "Base mode, always at the bottom of the mode stack."))
	TObjectPtr<UFVInteractorModeDefinition> DefaultMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modes", meta = (Tooltip = "Additional modes that can be pushed at runtime by ModeTag."))
	TArray<TObjectPtr<UFVInteractorModeDefinition>> Modes;

	const UFVInteractorModeDefinition* FindMode(const FGameplayTag& ModeTag) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
