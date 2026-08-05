// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVDialogueMessageTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVDialogueSet.h"
#include "FVDialogueDirector.generated.h"

class USUDSDialogue;
class UFVFlowNode_RunDialogue;
class ULevelSequencePlayer;
class UCineCameraComponent;

UCLASS()
class FLICKERVOIDNARRATIVE_API UFVDialogueDirector : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	static UFVDialogueDirector* Get(const UObject* WorldContext);
	
	virtual void Deinitialize() override;
	
	bool HasActiveDialogue() const { return ActiveDialogue != nullptr; }
	USUDSDialogue* GetActiveDialogue() const { return ActiveDialogue; }
	AActor* GetActiveNPC() const { return ActiveNPC.Get(); }
	AActor* GetActivePlayer() const { return ActivePlayer.Get(); }
	UFVDialogueSet* GetActiveSet() const { return ActiveSet; }
	
	void SetActiveDialogue(USUDSDialogue* Dialogue, AActor* NPC, AActor* Player, UFVDialogueSet* DialogueSet);
	void ClearActiveDialogue();
	void PersistActiveVariables();
	
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void PlayEnterSequence();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void BlendToShot(FName ShotName, AActor* FocusActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void RestoreGameplayCamera();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartContextualAction(FName ContextActionId);

protected:
	UPROPERTY() TObjectPtr<USUDSDialogue> ActiveDialogue;
	UPROPERTY() TWeakObjectPtr<AActor> ActiveNPC;
	UPROPERTY() TWeakObjectPtr<AActor> ActivePlayer;
	UPROPERTY() TObjectPtr<UFVDialogueSet> ActiveSet;
	
	UPROPERTY() TObjectPtr<ULevelSequencePlayer> SequencePlayer;
	UPROPERTY() TObjectPtr<AActor> RuntimeCameraActor;
	UPROPERTY() TObjectPtr<UFVDialogueContextualAction> ActiveContextualAction;
	
private:
	UFUNCTION() void OnEnterSequenceFinished();
};
