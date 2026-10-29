#include "Social/FVSocialLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Facts/FVFactDatabase.h"
#include "Identity/FVIdentityComponent.h"
#include "Scales/FVScaleDefinition.h"
#include "Scales/FVScaleLibrary.h"
#include "Social/FVFactionDefinition.h"
#include "Social/FVTitleDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialLibrary)

namespace FVSocial
{
	bool HasTag(const AActor* Actor, const FGameplayTag Tag)
	{
		if (!Actor || !Tag.IsValid())
		{
			return false;
		}
		const UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		return AbilitySystem && AbilitySystem->HasMatchingGameplayTag(Tag);
	}

	int32 Recognize(const AActor* Actor, const int32 Value)
	{
		if (!UFVSocialLibrary::IsDisguised(Actor))
		{
			return Value;
		}
		return FMath::RoundToInt(Value * UFVSocialSettings::Get().DisguiseRecognition);
	}

	int32 GetNotoriety(const AActor* Actor, const UFVFactionDefinition* Faction)
	{
		const UFVScaleDefinition* Global = UFVSocialLibrary::GetNotorietyScale();
		const int32 GlobalNotoriety = UFVScaleLibrary::GetValue(Actor, Global);
		const int32 FactionNotoriety = Faction ? UFVScaleLibrary::GetValue(Actor, Faction->Notoriety) : 0;
		return FMath::Max(FactionNotoriety, GlobalNotoriety);
	}
}

UFVScaleDefinition* UFVSocialLibrary::GetFameScale()
{
	return UFVSocialSettings::Get().Fame.LoadSynchronous();
}

UFVScaleDefinition* UFVSocialLibrary::GetNotorietyScale()
{
	return UFVSocialSettings::Get().Notoriety.LoadSynchronous();
}

UFVScaleDefinition* UFVSocialLibrary::GetRelationshipScale(const UFVCharacterDefinition* Character)
{
	const FFVCharacterFragment_Social* Social = Character ? Character->FindFragment<FFVCharacterFragment_Social>() : nullptr;
	return Social ? Social->Relationship.Get() : nullptr;
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

bool UFVSocialLibrary::IsDisguised(const AActor* Actor)
{
	return FVSocial::HasTag(Actor, UFVSocialSettings::Get().DisguiseRoot);
}

bool UFVSocialLibrary::IsDisguisedAs(const AActor* Actor, const UFVFactionDefinition* Faction)
{
	if (!Faction || !FVSocial::HasTag(Actor, Faction->DisguiseTag))
	{
		return false;
	}

	const int32 Notoriety = FVSocial::GetNotoriety(Actor, Faction);
	return Notoriety < Faction->DisguiseNotorietyLimit;
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

int32 UFVSocialLibrary::GetRecognizedFame(const AActor* Actor)
{
	const UFVScaleDefinition* Fame = GetFameScale();
	const int32 Value = UFVScaleLibrary::GetValue(Actor, Fame);
	return FVSocial::Recognize(Actor, Value);
}

int32 UFVSocialLibrary::GetRecognizedNotoriety(const AActor* Actor, const UFVFactionDefinition* Faction)
{
	const int32 Value = FVSocial::GetNotoriety(Actor, Faction);
	return FVSocial::Recognize(Actor, Value);
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

	const int32 Standing = UFVScaleLibrary::GetValue(Subject, Faction->Standing);
	return Faction->GetStandingAttitude(Standing);
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
		const UFVScaleDefinition* Relationship = GetRelationshipScale(Character);
		if (Relationship && Database->IsFactDefined(Relationship->Id))
		{
			Result.Add(Character);
		}
	}
	return Result;
}

bool UFVSocialLibrary::HasTitle(const UObject* WorldContext, const UFVTitleDefinition* Title)
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	return Database && Title && Title->Id.IsValid() && Database->GetFact(Title->Id) > 0;
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
	const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(Receiver);
	const UFVScaleDefinition* Relationship = GetRelationshipScale(Definition);
	if (!Relationship)
	{
		return 0;
	}

	const int32 Change = GetGiftRelationshipChange(Receiver, GiftTags);
	return UFVScaleLibrary::ModifyScale(WorldContext, Relationship, Change);
}
