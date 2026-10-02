#include "Facts/FVFactDatabase.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FVCoreRuntime.h"
#include "Facts/FVFactSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVFactDatabase)

UFVFactDatabase* UFVFactDatabase::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UFVFactDatabase>() : nullptr;
}

void UFVFactDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetToDefaults();
}

void UFVFactDatabase::ResetToDefaults()
{
	Facts.Reset();
	for (const FFVFactDefinition& Definition : GetDefault<UFVFactSettings>()->Definitions)
	{
		if (Definition.Tag.IsValid())
		{
			Facts.Add(Definition.Tag, Definition.bClamp ? FMath::Clamp(Definition.DefaultValue, Definition.Min, Definition.Max) : Definition.DefaultValue);
		}
	}
	LastChangedTag = FGameplayTag();
	OnFactsChanged.Broadcast();
}

FName UFVFactDatabase::GetFactValueName(FGameplayTag Tag) const
{
	return GetDefault<UFVFactSettings>()->GetValueName(Tag, GetFact(Tag));
}

void UFVFactDatabase::Deinitialize()
{
	Facts.Empty();
	FactChangedNative.Clear();
	OnFactsChanged.Clear();
	Super::Deinitialize();
}

int32 UFVFactDatabase::GetFact(FGameplayTag Tag) const
{
	const int32* Value = Facts.Find(Tag);
	return Value ? *Value : 0;
}

bool UFVFactDatabase::IsFactDefined(FGameplayTag Tag) const
{
	return Facts.Contains(Tag);
}

void UFVFactDatabase::SetFact(FGameplayTag Tag, int32 Value)
{
	WriteFact(Tag, Value);
}

void UFVFactDatabase::AddFact(FGameplayTag Tag, int32 Delta)
{
	WriteFact(Tag, GetFact(Tag) + Delta);
}

bool UFVFactDatabase::RemoveFact(FGameplayTag Tag)
{
	int32 OldValue = 0;
	if (!Facts.RemoveAndCopyValue(Tag, OldValue))
	{
		return false;
	}
	Notify(Tag, OldValue, 0);
	return true;
}

int32 UFVFactDatabase::RemoveFactsUnder(FGameplayTag Parent)
{
	TArray<FGameplayTag> ToRemove;
	for (const TPair<FGameplayTag, int32>& Pair : Facts)
	{
		if (Pair.Key.MatchesTag(Parent))
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& Tag : ToRemove)
	{
		RemoveFact(Tag);
	}
	return ToRemove.Num();
}

void UFVFactDatabase::ClearFacts()
{
	TArray<FGameplayTag> Tags;
	Facts.GetKeys(Tags);
	for (const FGameplayTag& Tag : Tags)
	{
		RemoveFact(Tag);
	}
}

void UFVFactDatabase::RestoreFacts(const TMap<FGameplayTag, int32>& InFacts)
{
	Facts = InFacts;
	LastChangedTag = FGameplayTag();
	OnFactsChanged.Broadcast();
}

void UFVFactDatabase::WriteFact(FGameplayTag Tag, int32 NewValue)
{
	if (!Tag.IsValid())
	{
		UE_LOG(LogFVFacts, Error, TEXT("Attempted to write an invalid fact tag."));
		return;
	}

	if (const FFVFactDefinition* Definition = GetDefault<UFVFactSettings>()->FindDefinition(Tag); Definition && Definition->bClamp)
	{
		NewValue = FMath::Clamp(NewValue, Definition->Min, Definition->Max);
	}

	int32& Stored = Facts.FindOrAdd(Tag, 0);
	const int32 OldValue = Stored;
	Stored = NewValue;
	Notify(Tag, OldValue, NewValue);
}

void UFVFactDatabase::Notify(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	LastChangedTag = Tag;
	FactChangedNative.Broadcast(Tag, OldValue, NewValue);
	OnFactsChanged.Broadcast();
}

#if !UE_BUILD_SHIPPING
static FAutoConsoleCommandWithWorldAndArgs GFVFactSet(
	TEXT("FV.Facts.Set"),
	TEXT("FV.Facts.Set <Tag> <Value>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UFVFactDatabase* Database = UFVFactDatabase::Get(World);
		if (!Database || Args.Num() < 2)
		{
			return;
		}
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(Args[0]), false);
		Database->SetFact(Tag, FCString::Atoi(*Args[1]));
	}));

static FAutoConsoleCommandWithWorld GFVFactDump(
	TEXT("FV.Facts.Dump"),
	TEXT("Prints all defined facts."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		const UFVFactDatabase* Database = UFVFactDatabase::Get(World);
		if (!Database)
		{
			return;
		}
		for (const TPair<FGameplayTag, int32>& Pair : Database->GetAllFacts())
		{
			UE_LOG(LogFVFacts, Display, TEXT("%s = %d"), *Pair.Key.ToString(), Pair.Value);
		}
	}));
#endif
