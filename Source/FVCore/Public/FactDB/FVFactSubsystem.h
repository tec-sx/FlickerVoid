#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactDB/FVFactTypes.h"
#include "FVFactSubsystem.generated.h"

class UFVFactSaveGame;

DECLARE_MULTICAST_DELEGATE_OneParam(FFactChanged, int32)
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAnyFactChanged, FGameplayTag, int32, EFVFactChangeReason)
DECLARE_MULTICAST_DELEGATE(FFactLoaded)

UCLASS()
class FLICKERVOIDCORE_API UFVFactSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	[[nodiscard]] static UFVFactSubsystem& Get(const UObject* WorldContextObject);

	virtual void Deinitialize() override;

	void ChangeFactValue(const FGameplayTag Tag, int32 NewValue, EFVFactValueChangeType ChangeType);
	// Sets a defined fact back to 0. The fact stays defined - use UndefineFact to remove it.
	void ResetFactValue(const FGameplayTag Tag);

	// Removes the fact entirely, so IsFactDefined returns false again. Returns true if it was defined.
	bool UndefineFact(const FGameplayTag Tag);

	// Removes every fact at or below ParentTag. Used for whole-quest resets. Returns the number removed.
	int32 UndefineFactsUnderTag(const FGameplayTag ParentTag);

	void ClearAllFacts();
	[[nodiscard]] bool GetFactValueIfDefined(const FGameplayTag Tag, int32& OutValue) const;
	[[nodiscard]] bool CheckFactCondition(const FFVFactCondition& Condition) const;
	[[nodiscard]] bool CheckFactConditions(const FFVFactConditionGroup& ConditionGroup) const;
	[[nodiscard]] bool IsFactDefined(const FGameplayTag Tag) const;

	FFactChanged& GetOnFactValueChangedDelegate(FGameplayTag Tag);
	FFactChanged& GetOnFactBecameDefinedDelegate(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "FactSubsystem")
	void OnGameSaved(UFVFactSaveGame* SaveGame) const;

	UFUNCTION(BlueprintCallable, Category = "FactSubsystem")
	void OnGameLoaded(const UFVFactSaveGame* SaveGame);

	FFactLoaded OnFactsLoaded;

	// Fires for every fact change regardless of tag. Lets tools observe the whole DB
	// with one subscription instead of one delegate per tag.
	FAnyFactChanged OnAnyFactChanged;

private:
	void BroadcastFactValueChanged(const FGameplayTag Tag, int32 Value,
								   EFVFactChangeReason Reason = EFVFactChangeReason::ValueChanged);
	void BroadcastFactDefined(const FGameplayTag Tag, int32 Value);
	void BroadcastFactUndefined(const FGameplayTag Tag);

private:
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, int32> DefinedFacts;

	// maybe merge them together, some struct?
	// also some form of map compaction
	TMap<FGameplayTag, FFactChanged> ValueDelegates;
	TMap<FGameplayTag, FFactChanged> DefinitionDelegates;

#if !UE_BUILD_SHIPPING
	static FAutoConsoleCommandWithWorldAndArgs ChangeFactValueCommand;
	static FAutoConsoleCommandWithWorldAndArgs GetFactValueCommand;
	static FAutoConsoleCommandWithWorld DumpFactsCommand;
#endif
};
