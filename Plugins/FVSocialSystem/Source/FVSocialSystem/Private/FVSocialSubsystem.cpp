#include "FVSocialSubsystem.h"

#include "Facts/FVFactDatabase.h"
#include "FVSocialStatics.h"
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
		FactChangedHandle = Facts->OnFactChangedNative().AddRaw(this, &UFVSocialSubsystem::HandleFactChanged);
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
	// Granting a title writes a fact, which calls back in here.
	if (bEvaluating)
	{
		return;
	}

	TGuardValue<bool> Guard(bEvaluating, true);

	FFVConditionContext Context;
	Context.WorldContext = this;

	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (Title == nullptr)
		{
			continue;
		}

		const bool bHeld = UFVSocialStatics::HasTitle(this, Title);
		const bool bEarned = Title->Conditions.Evaluate(Context);

		if (bEarned && !bHeld)
		{
			UFVSocialStatics::GrantTitle(this, Title);
			OnTitleEarned.Broadcast(Title);
		}
		else if (!bEarned && bHeld && Title->bTransient)
		{
			UFVSocialStatics::RevokeTitle(this, Title);
			OnTitleLost.Broadcast(Title);
		}
	}
}

TArray<const UFVTitleDefinition*> UFVSocialSubsystem::GetHeldTitles() const
{
	TArray<const UFVTitleDefinition*> Held;
	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (UFVSocialStatics::HasTitle(this, Title))
		{
			Held.Add(Title);
		}
	}
	return Held;
}
