#include "FactDB/FVFactSubsystem.h"

#include "GameplayTagContainer.h"
#include "FactDB/FVFactSaveGame.h"
#include "Logging/FVLogCategories.h"
#include "Logging/FVLogSystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FactDB/FVFactTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactSubsystem)

UFVFactSubsystem& UFVFactSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);

	UFVFactSubsystem* FactSubsystem = UGameInstance::GetSubsystem<UFVFactSubsystem>(World->GetGameInstance());
	check(FactSubsystem);

	return *FactSubsystem;
}

void UFVFactSubsystem::Deinitialize()
{
	// Facts and listeners are per-session; leaving them would leak stale state and
	// dangling subscribers across PIE runs.
	DefinedFacts.Empty();
	ValueDelegates.Empty();
	DefinitionDelegates.Empty();
	OnFactsLoaded.Clear();
	OnAnyFactChanged.Clear();

	Super::Deinitialize();
}

void UFVFactSubsystem::ChangeFactValue(const FGameplayTag Tag, int32 NewValue, EFVFactValueChangeType ChangeType)
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
		return;
	}

	auto GetUpdatedValue = [ ChangeType, NewValue ](const int32 Value)
	{
		switch (ChangeType)
		{
		case EFVFactValueChangeType::Set:
			return NewValue;
		case EFVFactValueChangeType::Add:
			return Value + NewValue;
		default:
			return 0;
		}
	};

	if (int32* CurrentValue = DefinedFacts.Find(Tag))
	{
		int32 UpdatedValue = GetUpdatedValue(*CurrentValue);
		if (*CurrentValue != UpdatedValue)
		{
			*CurrentValue = UpdatedValue;
			BroadcastFactValueChanged(Tag, *CurrentValue);
		}
	}
	else
	{
		int32& Value = DefinedFacts.Add(Tag);
		Value = GetUpdatedValue(Value);

		// first broadcast event, that fact became defined
		BroadcastFactDefined(Tag, Value);
		BroadcastFactValueChanged(Tag, Value, EFVFactChangeReason::Defined);
	}
}

void UFVFactSubsystem::ResetFactValue(const FGameplayTag Tag)
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
		return;
	}

	if (int32* CurrentValue = DefinedFacts.Find(Tag))
	{
		if (*CurrentValue != 0)
		{
			*CurrentValue = 0;
			BroadcastFactValueChanged(Tag, 0);
		}
	}
}

bool UFVFactSubsystem::UndefineFact(const FGameplayTag Tag)
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
		return false;
	}

	if (DefinedFacts.Remove(Tag) > 0)
	{
		BroadcastFactUndefined(Tag);
		return true;
	}

	return false;
}

int32 UFVFactSubsystem::UndefineFactsUnderTag(const FGameplayTag ParentTag)
{
	if (ParentTag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *ParentTag.ToString());
		return 0;
	}

	TArray<FGameplayTag> TagsToRemove;
	for (const TPair<FGameplayTag, int32>& Pair : DefinedFacts)
	{
		if (Pair.Key.MatchesTag(ParentTag))
		{
			TagsToRemove.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& Tag : TagsToRemove)
	{
		DefinedFacts.Remove(Tag);
		BroadcastFactUndefined(Tag);
	}

	return TagsToRemove.Num();
}

void UFVFactSubsystem::ClearAllFacts()
{
	TArray<FGameplayTag> TagsToRemove;
	DefinedFacts.GetKeys(TagsToRemove);

	DefinedFacts.Empty();

	for (const FGameplayTag& Tag : TagsToRemove)
	{
		BroadcastFactUndefined(Tag);
	}
}

bool UFVFactSubsystem::GetFactValueIfDefined(const FGameplayTag Tag, int32& OutValue) const
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
		return false;
	}

	if (const int32* TagValue = DefinedFacts.Find(Tag))
	{
		OutValue = *TagValue;
		return true;
	}
	return false;
}

bool UFVFactSubsystem::CheckFactCondition(const FFVFactCondition& Condition) const
{
	if (Condition.Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Condition.Tag.ToString());
		return false;
	}

	const int32* FactValue = DefinedFacts.Find(Condition.Tag);

	switch (Condition.Operator)
	{
	case EFVFactCompareOperator::Equals:
		return FactValue && *FactValue == Condition.WantedValue;
	case EFVFactCompareOperator::NotEquals:
		return FactValue && *FactValue != Condition.WantedValue;
	case EFVFactCompareOperator::Greater:
		return FactValue && *FactValue > Condition.WantedValue;
	case EFVFactCompareOperator::GreaterOrEqual:
		return FactValue && *FactValue >= Condition.WantedValue;
	case EFVFactCompareOperator::Less:
		return FactValue && *FactValue < Condition.WantedValue;
	case EFVFactCompareOperator::LessOrEqual:
		return FactValue && *FactValue <= Condition.WantedValue;
	case EFVFactCompareOperator::IsUndefined:
		return FactValue == nullptr;
	case EFVFactCompareOperator::IsDefined:
		return FactValue != nullptr;
	default:
		return false;
	}
}

bool UFVFactSubsystem::CheckFactConditions(const FFVFactConditionGroup& ConditionGroup) const
{
	if (ConditionGroup.Conditions.IsEmpty())
	{
		// An empty AllOf is vacuously true; an empty AnyOf has nothing to satisfy it.
		return ConditionGroup.LogicOp == EFVFactLogicOp::AllOf;
	}

	for (const FFVFactCondition& Condition : ConditionGroup.Conditions)
	{
		const bool bPassed = CheckFactCondition(Condition);

		if (ConditionGroup.LogicOp == EFVFactLogicOp::AllOf)
		{
			if (!bPassed)
			{
				return false;
			}
		}
		else if (bPassed)
		{
			return true;
		}
	}

	return ConditionGroup.LogicOp == EFVFactLogicOp::AllOf;
}

bool UFVFactSubsystem::IsFactDefined(const FGameplayTag Tag) const
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
		return false;
	}

	return DefinedFacts.Contains(Tag);
}

FFactChanged& UFVFactSubsystem::GetOnFactValueChangedDelegate(FGameplayTag Tag)
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
	}

	return ValueDelegates.FindOrAdd(Tag);
}

FFactChanged& UFVFactSubsystem::GetOnFactBecameDefinedDelegate(FGameplayTag Tag)
{
	if (Tag.IsValid() == false)
	{
		FV_LOG_ERROR(LogFVCore, "Passed fact tag %s is not valid", *Tag.ToString());
	}

	return DefinitionDelegates.FindOrAdd(Tag);
}

void UFVFactSubsystem::OnGameSaved(UFVFactSaveGame* SaveGame) const
{
	SaveGame->Facts = DefinedFacts;
}

void UFVFactSubsystem::OnGameLoaded(const UFVFactSaveGame* SaveGame)
{
	DefinedFacts = SaveGame->Facts;

	for (const TPair<FGameplayTag, int32>& Pair : DefinedFacts)
	{
		if (FFactChanged* Delegate = ValueDelegates.Find(Pair.Key))
		{
			Delegate->Broadcast(Pair.Value);
		}

		OnAnyFactChanged.Broadcast(Pair.Key, Pair.Value, EFVFactChangeReason::Loaded);
	}

	OnFactsLoaded.Broadcast();
}

void UFVFactSubsystem::BroadcastFactValueChanged(const FGameplayTag Tag, int32 Value, EFVFactChangeReason Reason)
{
	if (FFactChanged* Delegate = ValueDelegates.Find(Tag))
	{
		Delegate->Broadcast(Value);
	}

	// Single funnel for the global delegate, so listeners never see a duplicate for the define case.
	OnAnyFactChanged.Broadcast(Tag, Value, Reason);
}

void UFVFactSubsystem::BroadcastFactDefined(const FGameplayTag Tag, int32 Value)
{
	if (FFactChanged* Delegate = DefinitionDelegates.Find(Tag))
	{
		Delegate->Broadcast(Value);
	}
}

void UFVFactSubsystem::BroadcastFactUndefined(const FGameplayTag Tag)
{
	// Listeners see an undefined fact as value 0; use IsFactDefined to distinguish.
	BroadcastFactValueChanged(Tag, 0, EFVFactChangeReason::Undefined);
}

#if !UE_BUILD_SHIPPING
FAutoConsoleCommandWithWorldAndArgs UFVFactSubsystem::ChangeFactValueCommand
(
	TEXT("Facts.ChangeValue"),
	TEXT("Change value to provided one of a fact by given tag"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		// Facts.ChangeValue Fact.Tag Value ChangeType = Set
		if (Args.Num() < 2)
		{
			FV_LOG_ERROR(LogFVCore, "Incorrect number of arguments. Facts.ChangeValue Fact.Tag Value ChangeType = Set");
			return;
		}

		if (World)
		{
			UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(Args[0]));
			if (Tag.IsValid() == false)
			{
				FV_LOG_ERROR(LogFVCore, "Incorrect tag: %s", *Args[ 0 ]);
				return;
			}

			const FString& Value = Args[1];

			EFVFactValueChangeType ValueChangeType = EFVFactValueChangeType::Set;
			if (Args.IsValidIndex(2))
			{
				int64 EnumValue = StaticEnum<EFVFactValueChangeType>()->GetValueByNameString(Args[2]);
				if (EnumValue == INDEX_NONE)
				{
					FV_LOG_ERROR(LogFVCore, "Incorrect EFVFactValueChangeType value: %s", *Args[ 2 ]);
					return;
				}

				ValueChangeType = static_cast<EFVFactValueChangeType>(EnumValue);
			}

			int32 ValueFromString = 0;
			LexFromString(ValueFromString, *Value);

			FactSubsystem.ChangeFactValue(Tag, ValueFromString, ValueChangeType);
			FV_LOG_INFO(LogFVCore, "ChangeFactValue succeded");
		}
	})
);

FAutoConsoleCommandWithWorldAndArgs UFVFactSubsystem::GetFactValueCommand
(
	TEXT("Facts.GetValue"),
	TEXT("Prints value of a fact by given tag"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		// Facts.GetValue Fact.Tag
		if (Args.Num() < 1)
		{
			FV_LOG_ERROR(LogFVCore, "Incorrect number of arguments. Facts.GetValue Fact.Tag");
			return;
		}

		if (World)
		{
			UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);

			const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(Args[0]));
			if (Tag.IsValid() == false)
			{
				FV_LOG_ERROR(LogFVCore, "Incorrect tag: %s", *Tag.ToString());
				return;
			}

			int32 FactValue = 0;
			if (FactSubsystem.GetFactValueIfDefined(Tag, FactValue))
			{
				FV_LOG_INFO(LogFVCore, "%s: %d", *Tag.ToString(), FactValue);
				return;
			}

			FV_LOG_INFO(LogFVCore, "Fact %s is undefined", *Tag.ToString());
		}
	})
);

FAutoConsoleCommandWithWorld UFVFactSubsystem::DumpFactsCommand
(
	TEXT("Facts.Dump"),
	TEXT("Prints values of all defined facts"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (World)
		{
			UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
			FV_LOG_INFO(LogFVCore, "Dumping all defined facts");

			for (auto& [Tag, Value] : FactSubsystem.DefinedFacts)
			{
				FV_LOG_INFO(LogFVCore, "%s: %d", *Tag.ToString(), Value);
			}
		}
	})
);

#endif
