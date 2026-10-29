#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVSocialSubsystem.generated.h"

class UFVTitleDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnTitleChanged, const UFVTitleDefinition*, Title);

/** Grants and revokes titles as the facts behind them change. Nothing here runs on a timer. */
UCLASS()
class FVSOCIALSYSTEM_API UFVSocialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVSocialSubsystem* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	/** Re-checks every title; called on fact changes and usable after loading a save. */
	UFUNCTION(BlueprintCallable, Category = "FV|Social")
	void EvaluateTitles();

	UFUNCTION(BlueprintPure, Category = "FV|Social")
	TArray<UFVTitleDefinition*> GetHeldTitles() const;

	UPROPERTY(BlueprintAssignable, Category = "FV|Social")
	FFVOnTitleChanged OnTitleEarned;

	UPROPERTY(BlueprintAssignable, Category = "FV|Social")
	FFVOnTitleChanged OnTitleLost;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);
	void EvaluateTitlesOnce();

	/** A title another held title replaces stays away until that one is lost. */
	bool IsReplacedByHeldTitle(const UFVTitleDefinition* Title) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const UFVTitleDefinition>> Titles;

	FDelegateHandle FactChangedHandle;
	bool bEvaluating = false;
	bool bDirty = false;
};
