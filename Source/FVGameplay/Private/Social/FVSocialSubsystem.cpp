#include "Social/FVSocialSubsystem.h"

#include "Conditions/FVConditionLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "FVGameplayModule.h"
#include "FVGameplayTags.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemMessages.h"
#include "Social/FVSocialLibrary.h"
#include "Social/FVSocialTypes.h"
#include "Social/FVTitleDefinition.h"

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

	if (UGameInstance* GameInstance = InWorld.GetGameInstance())
	{
		GameInstance->GetOnPawnControllerChanged().AddDynamic(this, &UFVSocialSubsystem::HandlePawnControllerChanged);
	}

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(&InWorld);
	GiveListener = MessageSystem.RegisterListener(FVGameplayTags::Message_Item_GiveFinished, this, &UFVSocialSubsystem::HandleGiveFinished);

	EvaluateTitles();
}

void UFVSocialSubsystem::Deinitialize()
{
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->OnFactChangedNative().Remove(FactChangedHandle);
	}
	FactChangedHandle.Reset();

	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		GameInstance->GetOnPawnControllerChanged().RemoveDynamic(this, &UFVSocialSubsystem::HandlePawnControllerChanged);
	}

	GiveListener.Unregister();

	Super::Deinitialize();
}

void UFVSocialSubsystem::HandleFactChanged(FGameplayTag Tag, int32 OldValue, int32 NewValue)
{
	EvaluateTitles();
}

void UFVSocialSubsystem::HandlePawnControllerChanged(APawn* Pawn, AController* Controller)
{
	if (Pawn && Pawn->GetWorld() == GetWorld() && Controller && Controller->IsLocalPlayerController())
	{
		EvaluateTitles();
	}
}

void UFVSocialSubsystem::HandleGiveFinished(FGameplayTag Channel, const FFVItemGiveMessage& Message)
{
	if (!Message.bSucceeded || !Message.bAsGift)
	{
		return;
	}

	FGameplayTagContainer GiftTags;
	if (Message.Item && Message.Quantity > 0)
	{
		GiftTags.AppendTags(Message.Item->Tags);
		GiftTags.AddTag(Message.Item->Category);
	}

	UFVSocialLibrary::ReceiveGift(this, Message.Receiver, GiftTags);
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

	UE_CLOG(bDirty, LogFVGameplay, Warning, TEXT("Titles still changing after %d passes; check titles whose effects earn each other."), MaxPasses);
}

void UFVSocialSubsystem::EvaluateTitlesOnce()
{
	const FFVConditionContext Context = MakeTitleContext();

	// Ranking keeps the given order on ties, so the more recently current title wins a tie.
	TArray<const UFVTitleDefinition*> Candidates;
	for (const TObjectPtr<const UFVTitleDefinition>& Title : Titles)
	{
		if (Title != nullptr && Title->Id.IsValid())
		{
			Candidates.Add(Title);
		}
	}
	Candidates.StableSort([this](const UFVTitleDefinition& A, const UFVTitleDefinition& B) { return GetTitleRank(&A) > GetTitleRank(&B); });

	TArray<FFVConditionSet> Sets;
	Sets.Reserve(Candidates.Num());
	for (const UFVTitleDefinition* Title : Candidates)
	{
		Sets.Add(Title->Conditions);
	}

	const TArray<int32> Ranked = UFVConditionLibrary::RankConditionSets(Sets, Context);
	const UFVTitleDefinition* Best = Ranked.IsEmpty() ? nullptr : Candidates[Ranked[0]];

	TArray<const UFVTitleDefinition*> NewTitles;
	for (const int32 Index : Ranked)
	{
		if (!UFVSocialLibrary::HasTitle(this, Candidates[Index]))
		{
			NewTitles.Add(Candidates[Index]);
		}
	}

	// The first pass after a load only records the leader, so a title the story promoted stays current.
	if (Best != LeadingTitle.Get())
	{
		LeadingTitle = Best;
		if (Best != nullptr && (bTitlesTracked || !NewTitles.IsEmpty()))
		{
			PromoteTitle(Best);
		}
	}

	for (const UFVTitleDefinition* Title : NewTitles)
	{
		if (!UFVSocialLibrary::HasTitle(this, Title))
		{
			EarnTitle(Title);
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
		ApplyEarned(Title);
	}

	OnCurrentTitleChanged.Broadcast(Title);
}

void UFVSocialSubsystem::EarnTitle(const UFVTitleDefinition* Title)
{
	// Rank 1 places it below every title that has been current.
	if (UFVFactDatabase* Facts = UFVFactDatabase::Get(this))
	{
		Facts->SetFact(Title->Id, 1);
	}

	ApplyEarned(Title);
}

void UFVSocialSubsystem::ApplyEarned(const UFVTitleDefinition* Title)
{
	Title->OnEarned.Apply(MakeTitleContext());
	OnTitleEarned.Broadcast(Title);
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

FFVConditionContext UFVSocialSubsystem::MakeTitleContext() const
{
	return UFVConditionLibrary::MakePlayerContext(const_cast<UFVSocialSubsystem*>(this));
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
