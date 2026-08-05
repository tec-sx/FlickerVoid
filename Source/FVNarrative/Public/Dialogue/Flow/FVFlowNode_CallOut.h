// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "FVFlowNode_CallOut.generated.h"


struct FFVCallOutTableRow;

UCLASS(NotBlueprintable, meta = (DisplayName = "Play CallOut"))
class FLICKERVOIDNARRATIVE_API UFVFlowNode_CallOut : public UFlowNode
{
	GENERATED_BODY()
	
public:
	UFVFlowNode_CallOut();
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> CallOutDatabase;
	
	UPROPERTY(EditAnywhere) FGameplayTagContainer ContextTags;

	UPROPERTY(EditAnywhere) FGameplayTagContainer IdentityTags;

	UPROPERTY(EditAnywhere) bool bWaitForDuration = true;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

protected:
	void Play();
	bool IsRowValid(const FName& RowName, const FFVCallOutTableRow& Row, const FGameplayTagContainer& CombinedTags, const FGameplayTagContainer& NPCTags) const;
	bool PickRow(AActor* NPC, AActor* Player, FName& OutRowName, const FFVCallOutTableRow*& OutRow) const;
	void RememberPlayed(AActor* NPC, FName RowName);
	FName GetLastPlayed(AActor* NPC) const;

	AActor* ResolveOwnerActor() const;
	AActor* ResolvePlayerActor() const;
	void GatherTags(AActor* Actor, FGameplayTagContainer& OutTags) const;

	UFUNCTION()
	void OnCallOutFinished();

	FTimerHandle CallOutTimerHandle;
	TWeakObjectPtr<UAudioComponent> PlayingVoice;
	
	static TMap<TWeakObjectPtr<AActor>, FName> LastPlayedRowByNPC;

	static FName INPIN_In;
	static FName OUTPIN_Out;

#if WITH_EDITOR
public:
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif
};
