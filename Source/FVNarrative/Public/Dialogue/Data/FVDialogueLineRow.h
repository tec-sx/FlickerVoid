#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Dialogue/Conditions/FVDialogueConditionSet.h"
#include "FVDialogueLineRow.generated.h"

USTRUCT(BlueprintType)
struct FFVDialogueLineRow : public FTableRowBase
{
	GENERATED_BODY()

	// ── Identity ──────────────────────────────────────────────────────────────

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category="Identity",
		meta=(Categories="Dialogue.Speaker"))
	FGameplayTag SpeakerID;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category="Identity",
		meta=(Categories="Dialogue.Signal"))
	FGameplayTag Signal;

	// ── Content ───────────────────────────────────────────────────────────────

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category="Content",
		meta=(MultiLine="true"))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	TSoftObjectPtr<USoundBase> VoiceAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	TSoftObjectPtr<UAnimMontage> SpeakerMontage;

	// ── Availability ──────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Availability")
	FFVDialogueConditionSet Conditions;

	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Availability", 
		meta=(
			ClampMin="0",
			ToolTip="Use sparingly — prefer condition specificity for natural ranking."))
	int32 Priority = 0;
	
	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Selection", 
		meta=(
			ClampMin="0.01",
			ToolTip="Weight for random selection among lines with identical priority + score."))
	float Weight = 1.f;

	
	// ── Flow Control ──────────────────────────────────────────────────────────

	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Flow",
		meta=(ToolTip="If not empty, wait for input. If empty, advance to NextLineID. If both empty, end dialogue."))
	TArray<FName> ResponseOptionIDs;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category="Flow",
		meta=(ToolTip="Auto-advance. Only used when ResponseOptionIDs is empty. NAME_None ends the dialogue."))
	FName NextLineID = NAME_None;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category="Flow",
		meta=(
			Categories="Dialogue.Signal",
			ToolTip="Re-runs signal selection. Only use when NextLineID is NAME_None"))
	FGameplayTag NextSignal;

	// ── Lifetime ──────────────────────────────────────────────────────────────
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lifetime")
	bool bIsRepeatable = true;

	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Lifetime",
		meta=(EditCondition="!bIsRepeatable"))
	bool bConsumeOnPlay = false;

	// ── Events ────────────────────────────────────────────────────────────────
	
	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Events",
		meta=(
			Categories="Dialogue.Event",
			ToolTip="Optional gameplay event tag broadcast through GAS when line starts playing."))
	FGameplayTag EventOnPlay;
	
	UPROPERTY(
		EditAnywhere, 
		BlueprintReadWrite, 
		Category="Events",
		meta=(
			Categories="Dialogue.Event",
			ToolTip="Optional event broadcast when line completes. Useful for triggering cutscenes, spawning items."))
	FGameplayTag EventOnComplete;
};
