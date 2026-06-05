// Fill out your copyright notice in the Description page of Project Settings.

#include "Dialogue/FVDialogueConditionEvaluator.h"
#include "Subsystems/FVQuestSubsystem.h"
#include "Subsystems/FVWorldStateSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Attributes/FVCharacterAttributeSet.h"
#include "Attributes/FVProtagonistAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVDialogueConditionEvaluator)

UFVDialogueConditionEvaluator::UFVDialogueConditionEvaluator()
{
}

void UFVDialogueConditionEvaluator::Initialize(UWorld* World)
{
	CachedWorld = World;
	
	if (World && World->GetGameInstance())
	{
		WorldStateSubsystem = World->GetGameInstance()->GetSubsystem<UFVWorldStateSubsystem>();
		QuestSubsystem = World->GetGameInstance()->GetSubsystem<UFVQuestSubsystem>();
	}
}

FFVDialogueCondition UFVDialogueConditionEvaluator::ParseInlineCondition(const FString& ConditionString) const
{
	FFVDialogueCondition Result;

	TArray<FString> Parts;
	ConditionString.ParseIntoArray(Parts, TEXT(":"), true);

	if (Parts.Num() < 2)
	{
		return Result;
	}

	Result.ConditionType = FName(*Parts[0]);
	Result.TargetKey = Parts[1];

	if (Parts.Num() >= 3)
	{
		const FString& ThirdPart = Parts[2];

		// Check if it's a comparison operator
		if (ThirdPart == TEXT("==") || ThirdPart == TEXT("="))
		{
			Result.Comparison = EFVConditionComparison::Equal;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		else if (ThirdPart == TEXT("!="))
		{
			Result.Comparison = EFVConditionComparison::NotEqual;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		else if (ThirdPart == TEXT(">"))
		{
			Result.Comparison = EFVConditionComparison::GreaterThan;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		else if (ThirdPart == TEXT(">="))
		{
			Result.Comparison = EFVConditionComparison::GreaterThanOrEqual;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		else if (ThirdPart == TEXT("<"))
		{
			Result.Comparison = EFVConditionComparison::LessThan;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		else if (ThirdPart == TEXT("<="))
		{
			Result.Comparison = EFVConditionComparison::LessThanOrEqual;
			if (Parts.Num() >= 4)
			{
				Result.CompareValue = FCString::Atof(*Parts[3]);
			}
		}
		// Boolean state checks
		else if (ThirdPart.Equals(TEXT("Present"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("True"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("Completed"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("Discovered"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("Active"), ESearchCase::IgnoreCase))
		{
			Result.bRequireTagPresent = true;
		}
		else if (ThirdPart.Equals(TEXT("Absent"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("False"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("NotCompleted"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("NotDiscovered"), ESearchCase::IgnoreCase) ||
				 ThirdPart.Equals(TEXT("NotActive"), ESearchCase::IgnoreCase))
		{
			Result.bRequireTagPresent = false;
		}
		else
		{
			// Might be a target ID for relationship checks
			Result.TargetId = ThirdPart;
			if (Parts.Num() >= 5)
			{
				// Format: Type:Key:TargetId:Comparison:Value
				const FString& CompOp = Parts[3];
				if (CompOp == TEXT(">="))
				{
					Result.Comparison = EFVConditionComparison::GreaterThanOrEqual;
				}
				else if (CompOp == TEXT("<="))
				{
					Result.Comparison = EFVConditionComparison::LessThanOrEqual;
				}
				else if (CompOp == TEXT(">"))
				{
					Result.Comparison = EFVConditionComparison::GreaterThan;
				}
				else if (CompOp == TEXT("<"))
				{
					Result.Comparison = EFVConditionComparison::LessThan;
				}
				Result.CompareValue = FCString::Atof(*Parts[4]);
			}
		}
	}

	return Result;
}

bool UFVDialogueConditionEvaluator::EvaluateCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	bool bResult = false;

	const FString TypeString = Condition.ConditionType.ToString().ToLower();

	if (TypeString == TEXT("worldstate") || TypeString == TEXT("world"))
	{
		bResult = EvaluateWorldStateCondition(Condition, Context);
	}
	else if (TypeString == TEXT("relationship") || TypeString == TEXT("rel"))
	{
		bResult = EvaluateRelationshipCondition(Condition, Context);
	}
	else if (TypeString == TEXT("attribute") || TypeString == TEXT("attr"))
	{
		bResult = EvaluateAttributeCondition(Condition, Context);
	}
	else if (TypeString == TEXT("quest"))
	{
		bResult = EvaluateQuestCondition(Condition, Context);
	}
	else if (TypeString == TEXT("tag"))
	{
		bResult = EvaluateTagCondition(Condition, Context);
	}
	else if (TypeString == TEXT("memory") || TypeString == TEXT("mem"))
	{
		bResult = EvaluateMemoryCondition(Condition, Context);
	}
	else if (TypeString == TEXT("custom") || TypeString == TEXT("context"))
	{
		bResult = EvaluateCustomCondition(Condition, Context);
	}

	return Condition.bNegate ? !bResult : bResult;
}

bool UFVDialogueConditionEvaluator::EvaluateConditions(const TArray<FFVDialogueCondition>& Conditions, EFVConditionOperator Operator, const FFVDialogueContext& Context) const
{
	if (Conditions.Num() == 0)
	{
		return true; // No conditions = always pass
	}

	if (Operator == EFVConditionOperator::And)
	{
		for (const FFVDialogueCondition& Condition : Conditions)
		{
			if (!EvaluateCondition(Condition, Context))
			{
				return false;
			}
		}
		return true;
	}
	else // Or
	{
		for (const FFVDialogueCondition& Condition : Conditions)
		{
			if (EvaluateCondition(Condition, Context))
			{
				return true;
			}
		}
		return false;
	}
}

bool UFVDialogueConditionEvaluator::EvaluateLineConditions(const FFVDialogueLine& Line, const FFVDialogueContext& Context, const UDataTable* ConditionsTable) const
{
	TArray<FFVDialogueCondition> AllConditions;

	// Gather referenced conditions
	if (ConditionsTable)
	{
		for (const FName& ConditionId : Line.RequiredConditionIds)
		{
			if (const FFVDialogueConditionRow* Row = ConditionsTable->FindRow<FFVDialogueConditionRow>(ConditionId, TEXT("")))
			{
				AllConditions.Add(Row->Condition);
			}
		}
	}

	// Parse and add inline conditions
	for (const FString& InlineCondition : Line.InlineConditions)
	{
		FFVDialogueCondition Parsed = ParseInlineCondition(InlineCondition);
		if (Parsed.ConditionType != NAME_None)
		{
			AllConditions.Add(Parsed);
		}
	}

	return EvaluateConditions(AllConditions, Line.ConditionOperator, Context);
}

float UFVDialogueConditionEvaluator::CalculateSpecificityScore(const FFVDialogueLine& Line, const FFVDialogueContext& Context, const UDataTable* ConditionsTable) const
{
	float Score = static_cast<float>(Line.Priority);

	// Add weight for each condition that passes
	if (ConditionsTable)
	{
		for (const FName& ConditionId : Line.RequiredConditionIds)
		{
			if (const FFVDialogueConditionRow* Row = ConditionsTable->FindRow<FFVDialogueConditionRow>(ConditionId, TEXT("")))
			{
				if (EvaluateCondition(Row->Condition, Context))
				{
					Score += Row->Condition.Weight;
				}
			}
		}
	}

	// Inline conditions add 1.0 weight each
	for (const FString& InlineCondition : Line.InlineConditions)
	{
		FFVDialogueCondition Parsed = ParseInlineCondition(InlineCondition);
		if (Parsed.ConditionType != NAME_None && EvaluateCondition(Parsed, Context))
		{
			Score += 1.0f;
		}
	}

	return Score;
}

bool UFVDialogueConditionEvaluator::EvaluateWorldStateCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	if (!WorldStateSubsystem.IsValid())
	{
		return false;
	}

	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Condition.TargetKey), false);
	if (!Tag.IsValid())
	{
		// Try with WorldState prefix
		Tag = FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("WorldState.%s"), *Condition.TargetKey)), false);
	}

	if (Tag.IsValid())
	{
		bool bHasTag = WorldStateSubsystem->HasWorldStateTag(Tag);
		return bHasTag == Condition.bRequireTagPresent;
	}

	return false;
}

bool UFVDialogueConditionEvaluator::EvaluateRelationshipCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	if (!WorldStateSubsystem.IsValid())
	{
		return false;
	}

	FName NPCId = Condition.TargetId.IsEmpty() ? FName(*Condition.TargetKey) : FName(*Condition.TargetId);
	float Relationship = WorldStateSubsystem->GetNPCRelationship(NPCId);

	return CompareValues(Relationship, Condition.Comparison, Condition.CompareValue);
}

bool UFVDialogueConditionEvaluator::EvaluateAttributeCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	AActor* TargetActor = Context.ListenerActor.Get();
	if (!TargetActor)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!ASC)
	{
		return false;
	}

	// Try to find the attribute
	const FString& AttrName = Condition.TargetKey;
	float Value = 0.0f;
	bool bFound = false;

	// Check character attribute set
	if (const UFVCharacterAttributeSet* CharAttrs = ASC->GetSet<UFVCharacterAttributeSet>())
	{
		// Use reflection to find the attribute
		if (const FProperty* Prop = UFVCharacterAttributeSet::StaticClass()->FindPropertyByName(FName(*AttrName)))
		{
			FGameplayAttribute Attr(const_cast<FProperty*>(Prop));
			Value = ASC->GetNumericAttribute(Attr);
			bFound = true;
		}
	}

	// Check protagonist attribute set
	if (!bFound)
	{
		if (const UFVProtagonistAttributeSet* ProtagAttrs = ASC->GetSet<UFVProtagonistAttributeSet>())
		{
			if (const FProperty* Prop = UFVProtagonistAttributeSet::StaticClass()->FindPropertyByName(FName(*AttrName)))
			{
				FGameplayAttribute Attr(const_cast<FProperty*>(Prop));
				Value = ASC->GetNumericAttribute(Attr);
				bFound = true;
			}
		}
	}

	if (!bFound)
	{
		return false;
	}

	return CompareValues(Value, Condition.Comparison, Condition.CompareValue);
}

bool UFVDialogueConditionEvaluator::EvaluateQuestCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	if (!QuestSubsystem.IsValid())
	{
		return false;
	}

	FName QuestId = FName(*Condition.TargetKey);

	// Check based on the expected state
	if (Condition.bRequireTagPresent)
	{
		// Check if quest is completed or active
		if (Condition.TargetId.Equals(TEXT("Completed"), ESearchCase::IgnoreCase) ||
			Condition.TargetId.IsEmpty())
		{
			return QuestSubsystem->HasCompletedQuest(QuestId);
		}
		else if (Condition.TargetId.Equals(TEXT("Active"), ESearchCase::IgnoreCase))
		{
			return QuestSubsystem->GetQuestInstance(QuestId) != nullptr;
		}
		else if (Condition.TargetId.Equals(TEXT("Failed"), ESearchCase::IgnoreCase))
		{
			return QuestSubsystem->HasFailedQuest(QuestId);
		}
	}
	else
	{
		// Quest should NOT be in that state
		if (Condition.TargetId.Equals(TEXT("Completed"), ESearchCase::IgnoreCase) ||
			Condition.TargetId.IsEmpty())
		{
			return !QuestSubsystem->HasCompletedQuest(QuestId);
		}
		else if (Condition.TargetId.Equals(TEXT("Active"), ESearchCase::IgnoreCase))
		{
			return QuestSubsystem->GetQuestInstance(QuestId) == nullptr;
		}
	}

	return false;
}

bool UFVDialogueConditionEvaluator::EvaluateTagCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Condition.TargetKey), false);
	if (!Tag.IsValid())
	{
		return false;
	}

	// Check context tags first
	if (Context.ContextTags.HasTagExact(Tag))
	{
		return Condition.bRequireTagPresent;
	}

	// Check speaker's tags if available
	if (AActor* Speaker = Context.SpeakerActor.Get())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Speaker))
		{
			bool bHasTag = ASC->HasMatchingGameplayTag(Tag);
			return bHasTag == Condition.bRequireTagPresent;
		}
	}

	// Check listener's tags
	if (AActor* Listener = Context.ListenerActor.Get())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Listener))
		{
			bool bHasTag = ASC->HasMatchingGameplayTag(Tag);
			return bHasTag == Condition.bRequireTagPresent;
		}
	}

	return !Condition.bRequireTagPresent;
}

bool UFVDialogueConditionEvaluator::EvaluateMemoryCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	// Memory discovery is tracked via world state tags
	FGameplayTag MemoryTag = FGameplayTag::RequestGameplayTag(
		FName(*FString::Printf(TEXT("Memory.Discovered.%s"), *Condition.TargetKey)), false);

	if (WorldStateSubsystem.IsValid() && MemoryTag.IsValid())
	{
		bool bDiscovered = WorldStateSubsystem->HasWorldStateTag(MemoryTag);
		return bDiscovered == Condition.bRequireTagPresent;
	}

	return false;
}

bool UFVDialogueConditionEvaluator::EvaluateCustomCondition(const FFVDialogueCondition& Condition, const FFVDialogueContext& Context) const
{
	// Check custom context values
	const float* CustomValue = Context.CustomValues.Find(FName(*Condition.TargetKey));
	if (CustomValue)
	{
		return CompareValues(*CustomValue, Condition.Comparison, Condition.CompareValue);
	}

	// Check custom context strings for boolean checks
	const FString* CustomString = Context.CustomStrings.Find(FName(*Condition.TargetKey));
	if (CustomString)
	{
		bool bStringIsTrue = !CustomString->IsEmpty() && 
			!CustomString->Equals(TEXT("false"), ESearchCase::IgnoreCase) &&
			!CustomString->Equals(TEXT("0"));
		return bStringIsTrue == Condition.bRequireTagPresent;
	}

	return false;
}

bool UFVDialogueConditionEvaluator::CompareValues(float Value, EFVConditionComparison Comparison, float CompareValue) const
{
	switch (Comparison)
	{
	case EFVConditionComparison::Equal:
		return FMath::IsNearlyEqual(Value, CompareValue, KINDA_SMALL_NUMBER);
	case EFVConditionComparison::NotEqual:
		return !FMath::IsNearlyEqual(Value, CompareValue, KINDA_SMALL_NUMBER);
	case EFVConditionComparison::GreaterThan:
		return Value > CompareValue;
	case EFVConditionComparison::GreaterThanOrEqual:
		return Value >= CompareValue;
	case EFVConditionComparison::LessThan:
		return Value < CompareValue;
	case EFVConditionComparison::LessThanOrEqual:
		return Value <= CompareValue;
	default:
		return false;
	}
}