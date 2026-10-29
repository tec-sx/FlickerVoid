#include "FVSocialSubsystem.h"

#include "Facts/FVFactDatabase.h"
#include "FVSocialLibrary.h"
#include "FVSocialSystem.h"
#include "FVSocialTypes.h"
#include "FVTitleDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialSubsystem)

UFVSocialSubsystem* UFVSocialSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext != nullptr ? WorldContext->GetWorld() : nullptr;
	return World != nullptr ? World->GetSubsystem<UFVSocialSubsystem>() : nullptr;
}

bool UFVSocialSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVSocialSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	for (const TSoftObjectPtr<UFVTitleDefinition>& Soft : UFVSocialSettings::Get().Titles)
	{
		if (const UFVTitleDefinition* Title = Soft.LoadSynchronous())
		{
			Titles.Add(Title);
		}
	}

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		FactChangedHandle = Facts->OnFactChangedNative().AddUObject(this, &UFVSocialSubsystem::HandleFactChanged);
	}

	EvaluateTitles();
}

void UFVSocialSubsystem::Deinitialize()
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactChangedHandle);
	}
	FactChangedHandle.Reset();

	Super::Deinitialize();
}

void UFVSocialSubsystem::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	EvaluateTitles();
}

void UFVSocialSubsystem::EvaluateTitles()
{
	// Earning a title writes facts and runs its effects, which call back in here; those changes re-run the pass.
	if (bEvaluating)
	{
		bDirty = true;
		return;
	}

	TGuardValue<bool> Guard(bEvaluating, true);

	constexpr int32 MaxPasses = 8;
	int32 Pass = 0;
	do
	{
		bDirty = false;
		EvaluateTitlesOnce();
	}
	while (bDirty && ++Pass < MaxPasses);

	UE_CLOG(bDirty, LogFVSocialSystem, Warning, TEXT("Titles still changing after %d passes; check titles whose effects earn each other."), MaxPasses);
}

void UFVSocialSubsystem::EvaluateTitlesOnce()
{
	FFVConditionContext Context;
	Context.WorldContext = this;

	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (Title == nullptr)
		{
			continue;
		}

		if (!Title->Conditions.Evaluate(Context))
		{
			TitlesInCondition.Remove(Title);
			continue;
		}

		bool bAlreadyInCondition = false;
		TitlesInCondition.Add(Title, &bAlreadyInCondition);

		// A first pass only records which conditions hold, so held titles keep their order after a load.
		if (!UFVSocialLibrary::HasTitle(this, Title) || (bTitlesTracked && !bAlreadyInCondition))
		{
			PromoteTitle(Title);
		}
	}

	bTitlesTracked = true;
}

void UFVSocialSubsystem::PromoteTitle(const UFVTitleDefinition* Title)
{
	if (Title == nullptr || !Title->Id.IsValid() || GetCurrentTitle() == Title)
	{
		return;
	}

	const bool bFirstTime = !UFVSocialLibrary::HasTitle(this, Title);
	MakeCurrent(Title);

	if (bFirstTime)
	{
		FFVConditionContext Context;
		Context.WorldContext = this;
		Title->OnEarned.Apply(Context);
		OnTitleEarned.Broadcast(Title);
	}

	OnCurrentTitleChanged.Broadcast(Title);
}

void UFVSocialSubsystem::MakeCurrent(const UFVTitleDefinition* Title)
{
	// A title's fact holds its rank: the highest rank is the current title, 0 means not earned.
	int32 TopRank = 0;
	for (const TObjectPtr<const UFVTitleDefinition>& Other : Titles)
	{
		TopRank = FMath::Max(TopRank, GetTitleRank(Other));
	}

	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->SetFact(Title->Id, TopRank + 1);
	}
}

int32 UFVSocialSubsystem::GetTitleRank(const UFVTitleDefinition* Title) const
{
	const UFVFactDatabase* Facts = UFVFactDatabase::Get(this);
	return Facts && Title && Title->Id.IsValid() ? FMath::Max(0, Facts->GetFact(Title->Id)) : 0;
}

TArray<UFVTitleDefinition*> UFVSocialSubsystem::GetHeldTitles() const
{
	TArray<UFVTitleDefinition*> Held;
	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (GetTitleRank(Title) > 0)
		{
			Held.Add(const_cast<UFVTitleDefinition*>(Title.Get()));
		}
	}

	Held.Sort([this](const UFVTitleDefinition& A, const UFVTitleDefinition& B) { return GetTitleRank(&A) > GetTitleRank(&B); });
	return Held;
}

UFVTitleDefinition* UFVSocialSubsystem::GetCurrentTitle() const
{
	const TArray<UFVTitleDefinition*> Held = GetHeldTitles();
	return Held.IsEmpty() ? nullptr : Held[0];
}
