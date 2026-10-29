#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVSocialSubsystem.generated.h"

class UFVTitleDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFVOnTitleChanged, const UFVTitleDefinition*, Title);

/**
 * Keeps the player's titles: earned ones are never lost, and the most relevant one is current.
 * A title becomes current when it is first earned, when its conditions hold again after lapsing, or when the story promotes it.
 */
UCLASS()
class FVSOCIALSYSTEM_API UFVSocialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFVSocialSubsystem* Get(const UObject* WorldContext);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (ToolTip = "Re-checks every title; called on fact changes and usable after loading a save."))
	void EvaluateTitles();

	UFUNCTION(BlueprintCallable, Category = "FV|Social", meta = (ToolTip = "Makes the title current, earning it first (with its On Earned effects) if the player doesn't hold it yet."))
	void PromoteTitle(const UFVTitleDefinition* Title);

	UFUNCTION(BlueprintPure, Category = "FV|Social", meta = (ToolTip = "Held titles, current first, then from most to least recently current."))
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
	void EvaluateTitlesOnce();
	void MakeCurrent(const UFVTitleDefinition* Title);
	int32 GetTitleRank(const UFVTitleDefinition* Title) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const UFVTitleDefinition>> Titles;

	/** Titles whose conditions held on the last pass; a held title moving back in here becomes current. */
	TSet<const UFVTitleDefinition*> TitlesInCondition;

	FDelegateHandle FactChangedHandle;
	bool bEvaluating = false;
	bool bDirty = false;
	bool bTitlesTracked = false;
};
