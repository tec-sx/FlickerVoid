#pragma once

#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"

#include "FVPlayerState.generated.h"

class AFVPlayerController;
class UFVAbilitySystemComponent;
class UAbilitySystemComponent;
class UFVPawnData;

UCLASS(Config = Game)
class FLICKERVOID_API AFVPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AFVPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerState")
	AFVPlayerController* GetFVPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|PlayerState")
	UFVAbilitySystemComponent* GetFVAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FlickerVoid|PlayerState")
	UFVInputConfig* GetInputConfig() const;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	void SetPawnData(const UFVPawnData* InPawnData);

	static const FName NAME_FVAbilityReady;

protected:

	UPROPERTY()
	TObjectPtr<const UFVPawnData> PawnData;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "FV|PlayerState")
	TObjectPtr<UFVAbilitySystemComponent> AbilitySystemComponent;
};