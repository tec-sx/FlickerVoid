// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FVDialogueTypes.h"

#include "FVDialogueQuery.generated.h"

class UFVDialogueConditionEvaluator;

/**
 * FFVDialogueQueryResult
 * Result of a dialogue query
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueQueryResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FFVDialogueLine SelectedLine;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float SpecificityScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FFVDialogueLine> AllMatchingLines;
};

/**
 * UFVDialogueQuery
 * 
 * Queries dialogue tables to find the best matching line for a given signal and context.
 * Similar to CryEngine's DRS query system.
 */
UCLASS(BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVDialogueQuery : public UObject
{
	GENERATED_BODY()

public:
	UFVDialogueQuery();

	/** Initialize with a dialogue table and optional conditions table */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	void Initialize(UDataTable* InDialogueTable, UDataTable* InConditionsTable, UWorld* World);

	/**
	 * Query for the best matching dialogue line
	 * @param SignalName The signal/context that triggered this query
	 * @param Context Contextual information for condition evaluation
	 * @return Query result with the best matching line (if any)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	FFVDialogueQueryResult QueryDialogue(FName SignalName, const FFVDialogueContext& Context);

	/** Query for all matching dialogue lines (for debugging or special cases) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	TArray<FFVDialogueLine> QueryAllMatching(FName SignalName, const FFVDialogueContext& Context);

	/** Query for player response options */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	TArray<FFVDialogueResponse> QueryResponses(FName ParentSignal, const FFVDialogueContext& Context, UDataTable* ResponsesTable);

	/** Check if a response is available (conditions met) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	bool IsResponseAvailable(const FFVDialogueResponse& Response, const FFVDialogueContext& Context);

	/** Set dialogue table at runtime */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	void SetDialogueTable(UDataTable* InDialogueTable);

	/** Add additional dialogue table to query from */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Query")
	void AddDialogueTable(UDataTable* AdditionalTable);

private:
	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> DialogueTables;

	UPROPERTY()
	TObjectPtr<UDataTable> ConditionsTable;

	UPROPERTY()
	TObjectPtr<UFVDialogueConditionEvaluator> ConditionEvaluator;

	/** Cache of signal name to line mappings for faster lookup */
	TMultiMap<FName, FFVDialogueLine> SignalToLinesCache;

	void RebuildCache();
};