#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Conditions/FVCondition.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVSocialSubsystem.generated.h"

class AController;
class APawn;
class UFVTitleDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnTitleChanged, const UFVTitleDefinition*, Title);

/**
 * Keeps the player's titles: earned ones are never lost, and the most relevant one is current.
 * Among titles whose conditions hold, the hardest to earn leads (highest threshold, then furthest past it); the leader
 * becomes current whenever it changes. A title the story promotes stays current until the leader changes again.
 */
UCLASS()
class FVSOCIALSYSTEM_API UFVSocialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVSocialSubsystem* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (ToolTip = "Re-checks every title; called on fact changes. Call it when a non-fact value a title checks changes, such as an attribute."))
	void EvaluateTitles();

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (ToolTip = "Makes the title current, earning it first (with its On Earned effects) if the player doesn't hold it yet."))
	void PromoteTitle(const UFVTitleDefinition* Title);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Held titles, current first, then from most to least recently current, then those never current."))
	TArray<UFVTitleDefinition*> GetHeldTitles() const;

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "The title shown as the player's current one; null when none is held."))
	UFVTitleDefinition* GetCurrentTitle() const;

	UPROPERTY(BlueprintAssignable, Category = "FV|Social", meta = (ToolTip = "A title was earned for the first time."))
	FFVOnTitleChanged OnTitleEarned;

	UPROPERTY(BlueprintAssignable, Category = "FV|Social", meta = (ToolTip = "A different title became current, newly earned or one held before."))
	FFVOnTitleChanged OnCurrentTitleChanged;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue);

	/** Titles check the player pawn, which may spawn or change after world begin play. */
	UFUNCTION()
	void HandlePawnControllerChanged(APawn* Pawn, AController* Controller);

	void EvaluateTitlesOnce();
	void EarnTitle(const UFVTitleDefinition* Title);
	void ApplyEarned(const UFVTitleDefinition* Title);
	void MakeCurrent(const UFVTitleDefinition* Title);
	int32 GetTitleRank(const UFVTitleDefinition* Title) const;
	FFVConditionContext MakeTitleContext() const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const UFVTitleDefinition>> Titles;

	/** The best-scoring title on the last pass; when another one takes the lead it becomes current. */
	UPROPERTY(Transient)
	TObjectPtr<const UFVTitleDefinition> LeadingTitle;

	FDelegateHandle FactChangedHandle;
	bool bEvaluating = false;
	bool bDirty = false;
	bool bTitlesTracked = false;
};
