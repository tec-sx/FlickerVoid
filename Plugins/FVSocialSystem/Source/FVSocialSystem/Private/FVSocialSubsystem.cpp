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
	// Granting a title writes facts and runs its effects, which call back in here; those changes re-run the pass.
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

	UE_CLOG(bDirty, LogFVSocialSystem, Warning, TEXT("Titles still changing after %d passes; check titles whose effects earn or revoke each other."), MaxPasses);
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

		const bool bHeld = UFVSocialLibrary::HasTitle(this, Title);
		const bool bEarned = Title->Conditions.Evaluate(Context);

		if (bEarned && !bHeld && !IsReplacedByHeldTitle(Title))
		{
			UFVSocialLibrary::GrantTitle(this, Title);
			OnTitleEarned.Broadcast(Title);
		}
		else if (!bEarned && bHeld && Title->bTransient)
		{
			UFVSocialLibrary::RevokeTitle(this, Title);
			OnTitleLost.Broadcast(Title);
		}
	}
}

bool UFVSocialSubsystem::IsReplacedByHeldTitle(const UFVTitleDefinition* Title) const
{
	auto IsTitle = [Title](const TObjectPtr<UFVTitleDefinition>& Replaced) { return Replaced.Get() == Title; };

	for (const TObjectPtr<const UFVTitleDefinition>& Other : Titles)
	{
		if (Other != nullptr && Other != Title && Other->Replaces.ContainsByPredicate(IsTitle) && UFVSocialLibrary::HasTitle(this, Other))
		{
			return true;
		}
	}
	return false;
}

TArray<UFVTitleDefinition*> UFVSocialSubsystem::GetHeldTitles() const
{
	TArray<UFVTitleDefinition*> Held;
	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (UFVSocialLibrary::HasTitle(this, Title))
		{
			Held.Add(const_cast<UFVTitleDefinition*>(Title.Get()));
		}
	}
	return Held;
}
