#include "FVSocialLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Facts/FVFactDatabase.h"
#include "FVFactionDefinition.h"
#include "FVTitleDefinition.h"
#include "GameplayTagAssetInterface.h"
#include "Identity/FVIdentityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialLibrary)

namespace FVSocial
{
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

int32 UFVSocialLibrary::GetStanding(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	return Faction ? FVSocial::ReadFact(WorldContext, Faction->Id) : 0;
}

FGameplayTag UFVSocialLibrary::GetTier(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	const FFVReputationTier* Tier = Faction ? Faction->FindTier(GetStanding(WorldContext, Faction)) : nullptr;
	return Tier ? Tier->Tier : FGameplayTag();
}

void UFVSocialLibrary::ModifyStanding(UObject* WorldContext, const UFVFactionDefinition* Faction, const int32 Delta)
{
	if (!Faction || Delta == 0)
	{
		return;
	}

	// Only the change that applied feeds the rest, so rewards at max or min standing earn nothing more.
	const int32 Applied = AddClamped(WorldContext, Faction->Id, Delta, Faction->MinStanding, Faction->MaxStanding);
	if (Applied == 0 || (Applied > 0) != (Delta > 0))
	{
		return;
	}

	// Standing with any faction makes the player talked about, and standing with the wrong sort makes them wanted.
	ModifyFame(WorldContext, FMath::RoundToInt(FMath::Abs(Applied) * Faction->FameContribution));
	ModifyGlobalNotoriety(WorldContext, FMath::RoundToInt(Applied * Faction->NotorietyContribution));

	for (const TPair<TObjectPtr<UFVFactionDefinition>, float>& Relation : Faction->Relations)
	{
		if (Relation.Key && Relation.Key != Faction)
		{
			AddClamped(WorldContext, Relation.Key->Id, FMath::RoundToInt(Applied * Relation.Value), Relation.Key->MinStanding, Relation.Key->MaxStanding);
		}
	}
}

int32 UFVSocialLibrary::GetNotoriety(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
	return Faction ? FVSocial::ReadFact(WorldContext, Faction->NotorietyFact) : 0;
}

void UFVSocialLibrary::ModifyNotoriety(UObject* WorldContext, const UFVFactionDefinition* Faction, const int32 Delta)
{
	if (Faction)
	{
		AddClamped(WorldContext, Faction->NotorietyFact, Delta, 0, UFVSocialSettings::Get().MaxNotoriety);
	}
}

int32 UFVSocialLibrary::GetFame(const UObject* WorldContext)
{
	return FVSocial::ReadFact(WorldContext, UFVSocialSettings::Get().FameFact);
}

void UFVSocialLibrary::ModifyFame(UObject* WorldContext, const int32 Delta)
{
	AddClamped(WorldContext, UFVSocialSettings::Get().FameFact, Delta, 0, MAX_int32);
}

int32 UFVSocialLibrary::GetGlobalNotoriety(const UObject* WorldContext)
{
	return FVSocial::ReadFact(WorldContext, UFVSocialSettings::Get().NotorietyFact);
}

void UFVSocialLibrary::ModifyGlobalNotoriety(UObject* WorldContext, const int32 Delta)
{
	const UFVSocialSettings& Settings = UFVSocialSettings::Get();
	AddClamped(WorldContext, Settings.NotorietyFact, Delta, 0, Settings.MaxNotoriety);
}

bool UFVSocialLibrary::IsDisguised(const AActor* Actor)
{
	const FGameplayTag Root = UFVSocialSettings::Get().DisguiseRoot;
	return Root.IsValid() && FVSocial::HasTag(Actor, Root);
}

int32 UFVSocialLibrary::GetRecognizedFame(const AActor* Actor)
{
	const int32 Fame = GetFame(Actor);
	return IsDisguised(Actor) ? FMath::RoundToInt(Fame * UFVSocialSettings::Get().DisguiseRecognition) : Fame;
}

int32 UFVSocialLibrary::GetRecognizedNotoriety(const AActor* Actor, const UFVFactionDefinition* Faction)
{
	const int32 Notoriety = FMath::Max(GetNotoriety(Actor, Faction), GetGlobalNotoriety(Actor));
	return IsDisguised(Actor) ? FMath::RoundToInt(Notoriety * UFVSocialSettings::Get().DisguiseRecognition) : Notoriety;
}

bool UFVSocialLibrary::HasTitle(const UObject* WorldContext, const UFVTitleDefinition* Title)
{
	return Title != nullptr && FVSocial::ReadFact(WorldContext, Title->Id) > 0;
}

int32 UFVSocialLibrary::GetRelationship(const UObject* WorldContext, const UFVCharacterDefinition* Character)
{
	const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
	return Social ? FVSocial::ReadFact(WorldContext, Social->RelationshipFact) : 0;
}

void UFVSocialLibrary::ModifyRelationship(UObject* WorldContext, const UFVCharacterDefinition* Character, const int32 Delta)
{
	const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
	if (Social)
	{
		AddClamped(WorldContext, Social->RelationshipFact, Delta, MIN_int32, MAX_int32);
	}
}

int32 UFVSocialLibrary::GetGiftRelationshipChange(const AActor* Receiver, const FGameplayTagContainer& GiftTags)
{
	const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(Receiver);
	const FFVCharacterFragment_Gifts* Gifts = Definition ? Definition->FindFragment<FFVCharacterFragment_Gifts>() : nullptr;
	if (!Gifts)
	{
		return 0;
	}

	int32 Change = 0;
	bool bMatched = false;
	for (const FFVGiftPreference& Preference : Gifts->Preferences)
	{
		if (Preference.Tag.IsValid() && GiftTags.HasTag(Preference.Tag))
		{
			Change += Preference.RelationshipChange;
			bMatched = true;
		}
	}
	return bMatched ? Change : Gifts->DefaultRelationshipChange;
}

int32 UFVSocialLibrary::ReceiveGift(UObject* WorldContext, const AActor* Receiver, const FGameplayTagContainer& GiftTags)
{
	const FFVCharacterFragment_Social* Social = FindSocialFragment(Receiver);
	if (!Social || !Social->RelationshipFact.IsValid())
	{
		return 0;
	}

	const int32 Change = GetGiftRelationshipChange(Receiver, GiftTags);
	ModifyRelationship(WorldContext, UFVIdentityComponent::GetActorDefinition(Receiver), Change);
	return Change;
}

const FFVCharacterFragment_Social* UFVSocialLibrary::FindSocialFragment(const AActor* Actor)
{
	const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(Actor);
	return Definition ? Definition->FindFragment<FFVCharacterFragment_Social>() : nullptr;
}

UFVFactionDefinition* UFVSocialLibrary::GetActorFaction(const AActor* Actor)
{
	const FFVCharacterFragment_Social* Social = FindSocialFragment(Actor);
	return Social ? Social->Faction.Get() : nullptr;
}

bool UFVSocialLibrary::IsDisguisedAs(const AActor* Actor, const UFVFactionDefinition* Faction)
{
	return Faction
		&& FVSocial::HasTag(Actor, Faction->DisguiseTag)
		&& FMath::Max(GetNotoriety(Actor, Faction), GetGlobalNotoriety(Actor)) < Faction->DisguiseNotorietyLimit;
}

TArray<UFVFactionDefinition*> UFVSocialLibrary::GetFactions()
{
	TArray<UFVFactionDefinition*> Result;
	for (const TSoftObjectPtr<UFVFactionDefinition>& Soft : UFVSocialSettings::Get().Factions)
	{
		if (UFVFactionDefinition* Faction = Soft.LoadSynchronous())
		{
			Result.Add(Faction);
		}
	}
	return Result;
}

UFVFactionDefinition* UFVSocialLibrary::GetDisguiseFaction(const AActor* Actor)
{
	for (UFVFactionDefinition* Faction : GetFactions())
	{
		if (IsDisguisedAs(Actor, Faction))
		{
			return Faction;
		}
	}
	return nullptr;
}

TArray<UFVCharacterDefinition*> UFVSocialLibrary::GetKnownCharacters(const UObject* WorldContext)
{
	TArray<UFVCharacterDefinition*> Result;
	const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database)
	{
		return Result;
	}

	for (const TSoftObjectPtr<UFVCharacterDefinition>& Soft : UFVSocialSettings::Get().Relationships)
	{
		UFVCharacterDefinition* Character = Soft.LoadSynchronous();
		const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
		if (Social && Database->IsFactDefined(Social->RelationshipFact))
		{
			Result.Add(Character);
		}
	}
	return Result;
}

EFVAttitude UFVSocialLibrary::GetAttitude(const UFVFactionDefinition* Faction, const AActor* Subject)
{
	if (!Faction || !Subject)
	{
		return EFVAttitude::Neutral;
	}

	if (GetActorFaction(Subject) == Faction || IsDisguisedAs(Subject, Faction))
	{
		return EFVAttitude::Allied;
	}

	if (GetRecognizedNotoriety(Subject, Faction) >= Faction->HostileNotoriety)
	{
		return EFVAttitude::Hostile;
	}

	const FFVReputationTier* Tier = Faction->FindTier(GetStanding(Subject, Faction));
	return Tier ? Tier->Attitude : EFVAttitude::Neutral;
}

int32 UFVSocialLibrary::AddClamped(UObject* WorldContext, const FGameplayTag Fact, const int32 Delta, const int32 Min, const int32 Max)
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database || !Fact.IsValid() || Delta == 0)
	{
		return 0;
	}

	const int32 Current = Database->GetFact(Fact);
	const int32 Next = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(Current) + Delta, Min, Max));
	Database->SetFact(Fact, Next);
	return static_cast<int32>(static_cast<int64>(Next) - Current);
}
