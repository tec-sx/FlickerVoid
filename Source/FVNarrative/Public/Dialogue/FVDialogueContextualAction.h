#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FVDialogueContextualAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContextualActionFinished, bool, bSuccess);

UCLASS(Abstract, Blueprintable, BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVDialogueContextualAction : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue")
	void StartAction(AActor* Player, AActor* NPC, FName ActionId);
	virtual void StartAction_Implementation(AActor* Player, AActor* NPC, FName ActionId);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void FinishAction(bool bSuccess = true);

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnContextualActionFinished OnFinished;

protected:
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> PlayerActor;
	UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> NPCActor;
	UPROPERTY(BlueprintReadOnly) FName CurrentActionId;
};
