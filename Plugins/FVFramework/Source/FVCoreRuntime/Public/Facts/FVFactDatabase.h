#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FVFactDatabase.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FFVOnFactChangedNative, FGameplayTag /*Tag*/, int32 /*OldValue*/, int32 /*NewValue*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnFactsChanged);

UCLASS()
class FVCORERUNTIME_API UFVFactDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UFVFactDatabase* Get(const UObject* WorldContext);

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Facts", meta = (GameplayTagFilter = "Fact"))
	int32 GetFact(FGameplayTag Tag) const;

	UFUNCTION(BlueprintPure, Category = "Facts")
	bool IsFactDefined(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Facts")
	void SetFact(FGameplayTag Tag, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Facts")
	void AddFact(FGameplayTag Tag, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Facts")
	bool RemoveFact(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Facts")
	int32 RemoveFactsUnder(FGameplayTag Parent);

	UFUNCTION(BlueprintCallable, Category = "Facts")
	void ClearFacts();

	UFUNCTION(BlueprintPure, Category = "Facts")
	FGameplayTag GetLastChangedTag() const { return LastChangedTag; }

	const TMap<FGameplayTag, int32>& GetAllFacts() const { return Facts; }
	void RestoreFacts(const TMap<FGameplayTag, int32>& InFacts);

	FFVOnFactChangedNative& OnFactChangedNative() { return FactChangedNative; }

	UPROPERTY(BlueprintAssignable, Category = "Facts")
	FFVOnFactsChanged OnFactsChanged;

private:
	void WriteFact(FGameplayTag Tag, int32 NewValue);
	void Notify(FGameplayTag Tag, int32 OldValue, int32 NewValue);

	UPROPERTY(SaveGame)
	TMap<FGameplayTag, int32> Facts;

	FGameplayTag LastChangedTag;
	FFVOnFactChangedNative FactChangedNative;
};
