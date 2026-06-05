// Fill out your copyright notice in the Description page of Project Settings.

#include "Dialogue/FVDialogueQuery.h"
#include "Dialogue/FVDialogueConditionEvaluator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueQuery)

UFVDialogueQuery::UFVDialogueQuery()
{
}

void UFVDialogueQuery::Initialize(UDataTable* InDialogueTable, UDataTable* InConditionsTable, UWorld* World)
{
	DialogueTables.Empty();
	if (InDialogueTable)
	{
		DialogueTables.Add(InDialogueTable);
	}

	ConditionsTable = InConditionsTable;

	ConditionEvaluator = NewObject<UFVDialogueConditionEvaluator>(this);
	ConditionEvaluator->Initialize(World);

	RebuildCache();
}

void UFVDialogueQuery::SetDialogueTable(UDataTable* InDialogueTable)
{
	DialogueTables.Empty();
	if (InDialogueTable)
	{
		DialogueTables.Add(InDialogueTable);
	}
	RebuildCache();
}

void UFVDialogueQuery::AddDialogueTable(UDataTable* AdditionalTable)
{
	if (AdditionalTable && !DialogueTables.Contains(AdditionalTable))
	{
		DialogueTables.Add(AdditionalTable);
		RebuildCache();
	}
}

void UFVDialogueQuery::RebuildCache()
{
	SignalToLinesCache.Empty();

	for (UDataTable* Table : DialogueTables)
	{
		if (!Table)
		{
			continue;
		}

		const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
		for (const auto& Pair : RowMap)
		{
			const FFVDialogueLine* Line = reinterpret_cast<const FFVDialogueLine*>(Pair.Value);
			if (Line && Line->SignalName != NAME_None)
			{
				SignalToLinesCache.Add(Line->SignalName, *Line);
			}
		}
	}
}

FFVDialogueQueryResult UFVDialogueQuery::QueryDialogue(FName SignalName, const FFVDialogueContext& Context)
{
	FFVDialogueQueryResult Result;

	if (!ConditionEvaluator || SignalName == NAME_None)
	{
		return Result;
	}

	// Get all lines for this signal
	TArray<FFVDialogueLine> CandidateLines;
	SignalToLinesCache.MultiFind(SignalName, CandidateLines);

	// Filter by speaker if context has one
	if (Context.SpeakerId != NAME_None)
	{
		CandidateLines.RemoveAll([&Context](const FFVDialogueLine& Line)
		{
			return Line.SpeakerId != NAME_None && Line.SpeakerId != Context.SpeakerId;
		});
	}

	// Evaluate conditions and find best match
	float BestScore = -FLT_MAX;
	const FFVDialogueLine* BestLine = nullptr;

	for (const FFVDialogueLine& Line : CandidateLines)
	{
		if (ConditionEvaluator->EvaluateLineConditions(Line, Context, ConditionsTable))
		{
			Result.AllMatchingLines.Add(Line);

			float Score = ConditionEvaluator->CalculateSpecificityScore(Line, Context, ConditionsTable);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestLine = &Line;
			}
		}
	}

	if (BestLine)
	{
		Result.bSuccess = true;
		Result.SelectedLine = *BestLine;
		Result.SpecificityScore = BestScore;
	}

	return Result;
}

TArray<FFVDialogueLine> UFVDialogueQuery::QueryAllMatching(FName SignalName, const FFVDialogueContext& Context)
{
	FFVDialogueQueryResult Result = QueryDialogue(SignalName, Context);
	return Result.AllMatchingLines;
}

TArray<FFVDialogueResponse> UFVDialogueQuery::QueryResponses(FName ParentSignal, const FFVDialogueContext& Context, UDataTable* ResponsesTable)
{
	TArray<FFVDialogueResponse> Results;

	if (!ResponsesTable || !ConditionEvaluator)
	{
		return Results;
	}

	const TMap<FName, uint8*>& RowMap = ResponsesTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FFVDialogueResponse* Response = reinterpret_cast<const FFVDialogueResponse*>(Pair.Value);
		if (Response && Response->ParentSignal == ParentSignal)
		{
			// Check if response should be shown (either available or show when unavailable)
			bool bAvailable = IsResponseAvailable(*Response, Context);
			if (bAvailable || Response->bShowWhenUnavailable)
			{
				Results.Add(*Response);
			}
		}
	}

	// Sort by sort order
	Results.Sort([](const FFVDialogueResponse& A, const FFVDialogueResponse& B)
	{
		return A.SortOrder < B.SortOrder;
	});

	return Results;
}

bool UFVDialogueQuery::IsResponseAvailable(const FFVDialogueResponse& Response, const FFVDialogueContext& Context)
{
	if (!ConditionEvaluator)
	{
		return true;
	}

	// Parse and evaluate inline conditions
	TArray<FFVDialogueCondition> Conditions;
	for (const FString& InlineCondition : Response.InlineConditions)
	{
		FFVDialogueCondition Parsed = ConditionEvaluator->ParseInlineCondition(InlineCondition);
		if (Parsed.ConditionType != NAME_None)
		{
			Conditions.Add(Parsed);
		}
	}

	return ConditionEvaluator->EvaluateConditions(Conditions, EFVConditionOperator::And, Context);
}