#include "FVSocialStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Facts/FVFactDatabase.h"
#include "FVFactionDefinition.h"
#include "GameplayTagAssetInterface.h"
#include "Identity/FVIdentityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialStatics)

namespace FVSocial
{
	constexpr int32 MaxNotoriety = 100;

	int32 ReadFact(const UObject* WorldContext, const FGameplayTag Fact)
	{
		const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
		return Database && Fact.IsValid() ? Database->GetFact(Fact) : 0;
	}

	bool HasTag(const AActor* Actor, const FGameplayTag Tag)
	{
		if (!Actor || !Tag.IsValid())
		{
			return false;
		}
		if (const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			return ASC->HasMatchingGameplayTag(Tag);
		}
		const IGameplayTagAssetInterface* TagOwner = Cast<IGameplayTagAssetInterface>(Actor);
		return TagOwner && TagOwner->HasMatchingGameplayTag(Tag);
	}
}

const FFVReputationTier* UFVFactionDefinition::FindTier(const int32 Standing) const
{
	const FFVReputationTier* Result = nullptr;
	for (const FFVReputationTier& Tier : Tiers)
	{
		if (Standing >= Tier.MinStanding)
		{
			Result = &Tier;
		}
	}
	return Result;
}

int32 UFVSocialStatics::GetStanding(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	return Faction ? FVSocial::ReadFact(WorldContext, Faction->Id) : 0;
}

FGameplayTag UFVSocialStatics::GetTier(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	const FFVReputationTier* Tier = Faction ? Faction->FindTier(GetStanding(WorldContext, Faction)) : nullptr;
	return Tier ? Tier->Tier : FGameplayTag();
}

void UFVSocialStatics::ModifyStanding(UObject* WorldContext, const UFVFactionDefinition* Faction, const int32 Delta)
{
	if (!Faction || Delta == 0)
	{
		return;
	}

	AddClamped(WorldContext, Faction->Id, Delta, Faction->MinStanding, Faction->MaxStanding);
	for (const TPair<TObjectPtr<UFVFactionDefinition>, float>& Relation : Faction->Relations)
	{
		if (Relation.Key && Relation.Key != Faction)
		{
			AddClamped(WorldContext, Relation.Key->Id, FMath::RoundToInt(Delta * Relation.Value), Relation.Key->MinStanding, Relation.Key->MaxStanding);
		}
	}
}

int32 UFVSocialStatics::GetNotoriety(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	return Faction ? FVSocial::ReadFact(WorldContext, Faction->NotorietyFact) : 0;
}

void UFVSocialStatics::ModifyNotoriety(UObject* WorldContext, const UFVFactionDefinition* Faction, const int32 Delta)
{
	if (Faction)
	{
		AddClamped(WorldContext, Faction->NotorietyFact, Delta, 0, FVSocial::MaxNotoriety);
	}
}

int32 UFVSocialStatics::GetPersonalReputation(const UObject* WorldContext)
{
	return FVSocial::ReadFact(WorldContext, GetDefault<UFVSocialSettings>()->PersonalReputationFact);
}

void UFVSocialStatics::ModifyPersonalReputation(UObject* WorldContext, const int32 Delta)
{
	AddClamped(WorldContext, GetDefault<UFVSocialSettings>()->PersonalReputationFact, Delta, MIN_int32, MAX_int32);
}

int32 UFVSocialStatics::GetRelationship(const UObject* WorldContext, const UFVCharacterDefinition* Character)
{
	const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
	return Social ? FVSocial::ReadFact(WorldContext, Social->RelationshipFact) : 0;
}

void UFVSocialStatics::ModifyRelationship(UObject* WorldContext, const UFVCharacterDefinition* Character, const int32 Delta)
{
	const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
	if (Social)
	{
		AddClamped(WorldContext, Social->RelationshipFact, Delta, MIN_int32, MAX_int32);
	}
}

const FFVCharacterFragment_Social* UFVSocialStatics::FindSocialFragment(const AActor* Actor)
{
	const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(Actor);
	return Definition ? Definition->FindFragment<FFVCharacterFragment_Social>() : nullptr;
}

UFVFactionDefinition* UFVSocialStatics::GetActorFaction(const AActor* Actor)
{
	const FFVCharacterFragment_Social* Social = FindSocialFragment(Actor);
	return Social ? Social->Faction.Get() : nullptr;
}

bool UFVSocialStatics::IsDisguisedAs(const AActor* Actor, const UFVFactionDefinition* Faction)
{
	return Faction
		&& FVSocial::HasTag(Actor, Faction->DisguiseTag)
		&& GetNotoriety(Actor, Faction) < Faction->DisguiseNotorietyLimit;
}

EFVAttitude UFVSocialStatics::GetAttitude(const UFVFactionDefinition* Faction, const AActor* Subject)
{
	if (!Faction || !Subject)
	{
		return EFVAttitude::Neutral;
	}

	if (GetActorFaction(Subject) == Faction || IsDisguisedAs(Subject, Faction))
	{
		return EFVAttitude::Allied;
	}

	if (GetNotoriety(Subject, Faction) >= Faction->HostileNotoriety)
	{
		return EFVAttitude::Hostile;
	}

	const FFVReputationTier* Tier = Faction->FindTier(GetStanding(Subject, Faction));
	return Tier ? Tier->Attitude : EFVAttitude::Neutral;
}

void UFVSocialStatics::AddClamped(UObject* WorldContext, const FGameplayTag Fact, const int32 Delta, const int32 Min, const int32 Max)
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database || !Fact.IsValid() || Delta == 0)
	{
		return;
	}

	const int64 Next = static_cast<int64>(Database->GetFact(Fact)) + Delta;
	Database->SetFact(Fact, static_cast<int32>(FMath::Clamp<int64>(Next, Min, Max)));
}
