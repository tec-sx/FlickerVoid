#include "FVSocialConditions.h"

#include "FVFactionDefinition.h"
#include "FVSocialLibrary.h"
#include "FVTitleDefinition.h"
#include "Identity/FVIdentityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialConditions)

#define LOCTEXT_NAMESPACE "FVSocialConditions"

namespace FVSocialConditions
{
	bool Compare(const int32 Current, const EFVFactCompare Op, const int32 Value)
	{
		switch (Op)
		{
		case EFVFactCompare::Equal:          return Current == Value;
		case EFVFactCompare::NotEqual:       return Current != Value;
		case EFVFactCompare::Greater:        return Current > Value;
		case EFVFactCompare::GreaterOrEqual: return Current >= Value;
		case EFVFactCompare::Less:           return Current < Value;
		case EFVFactCompare::LessOrEqual:    return Current <= Value;
		default:                             return false;
		}
	}

	FText Op(const EFVFactCompare Compare)
	{
		return StaticEnum<EFVFactCompare>()->GetDisplayNameTextByValue(static_cast<int64>(Compare));
	}

	FText Name(const UFVDefinition* Definition)
	{
		return Definition ? Definition->Display.Name : LOCTEXT("None", "<none>");
	}

	AActor* Pick(const FFVConditionContext& Context, const EFVContextActor Which)
	{
		return Which == EFVContextActor::Instigator ? Context.Instigator.Get() : Context.Target.Get();
	}

	const UFVCharacterDefinition* CharacterOrTarget(const UFVCharacterDefinition* Character, const FFVConditionContext& Context)
	{
		return Character ? Character : UFVIdentityComponent::GetActorDefinition(Context.Target);
	}
}

bool FFVCondition_Standing::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Faction && FVSocialConditions::Compare(UFVSocialLibrary::GetStanding(Context.WorldContext, Faction), Compare, Value);
}

FText FFVCondition_Standing::GetDescription() const
{
	return FText::Format(LOCTEXT("Standing", "{0} standing {1} {2}"), FVSocialConditions::Name(Faction), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Notoriety::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Faction && FVSocialConditions::Compare(UFVSocialLibrary::GetNotoriety(Context.WorldContext, Faction), Compare, Value);
}

FText FFVCondition_Notoriety::GetDescription() const
{
	return FText::Format(LOCTEXT("Notoriety", "{0} notoriety {1} {2}"), FVSocialConditions::Name(Faction), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Fame::EvaluateImpl(const FFVConditionContext& Context) const
{
	const int32 Fame = bIgnoreDisguise
		? UFVSocialLibrary::GetFame(Context.WorldContext)
		: UFVSocialLibrary::GetRecognizedFame(Context.Instigator);

	return FVSocialConditions::Compare(Fame, Compare, Value);
}

FText FFVCondition_Fame::GetDescription() const
{
	return FText::Format(LOCTEXT("Fame", "Fame {0} {1}"), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_GlobalNotoriety::EvaluateImpl(const FFVConditionContext& Context) const
{
	return FVSocialConditions::Compare(UFVSocialLibrary::GetGlobalNotoriety(Context.WorldContext), Compare, Value);
}

FText FFVCondition_GlobalNotoriety::GetDescription() const
{
	return FText::Format(LOCTEXT("GlobalNotoriety", "Notoriety {0} {1}"), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_HasTitle::EvaluateImpl(const FFVConditionContext& Context) const
{
	return UFVSocialLibrary::HasTitle(Context.WorldContext, Title);
}

FText FFVCondition_HasTitle::GetDescription() const
{
	return Title != nullptr ? Title->Display.Name : FText::GetEmpty();
}

bool FFVCondition_Relationship::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVCharacterDefinition* Resolved = FVSocialConditions::CharacterOrTarget(Character, Context);
	return Resolved && FVSocialConditions::Compare(UFVSocialLibrary::GetRelationship(Context.WorldContext, Resolved), Compare, Value);
}

FText FFVCondition_Relationship::GetDescription() const
{
	const FText Who = Character ? Character->Display.Name : LOCTEXT("Target", "target");
	return FText::Format(LOCTEXT("Relationship", "Relationship with {0} {1} {2}"), Who, FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Attitude::EvaluateImpl(const FFVConditionContext& Context) const
{
	const AActor* ObserverActor = FVSocialConditions::Pick(Context, Observer);
	const AActor* Subject = FVSocialConditions::Pick(Context, Observer == EFVContextActor::Target ? EFVContextActor::Instigator : EFVContextActor::Target);
	const UFVFactionDefinition* ObserverFaction = Faction ? Faction.Get() : UFVSocialLibrary::GetActorFaction(ObserverActor);
	return ObserverFaction && UFVSocialLibrary::GetAttitude(ObserverFaction, Subject) >= AtLeast;
}

FText FFVCondition_Attitude::GetDescription() const
{
	const FText Attitude = StaticEnum<EFVAttitude>()->GetDisplayNameTextByValue(static_cast<int64>(AtLeast));
	return FText::Format(LOCTEXT("Attitude", "Attitude at least {0}"), Attitude);
}

bool FFVCondition_IsDisguised::EvaluateImpl(const FFVConditionContext& Context) const
{
	return UFVSocialLibrary::IsDisguisedAs(FVSocialConditions::Pick(Context, Subject), Faction);
}

FText FFVCondition_IsDisguised::GetDescription() const
{
	return FText::Format(LOCTEXT("Disguised", "Disguised as {0}"), FVSocialConditions::Name(Faction));
}

void FFVEffect_ModifyStanding::Apply(const FFVConditionContext& Context) const
{
	UFVSocialLibrary::ModifyStanding(Context.WorldContext, Faction, Delta);
}

FText FFVEffect_ModifyStanding::GetDescription() const
{
	return FText::Format(LOCTEXT("ModStanding", "{0} standing {1}"), FVSocialConditions::Name(Faction), FText::AsNumber(Delta));
}

void FFVEffect_ModifyNotoriety::Apply(const FFVConditionContext& Context) const
{
	UFVSocialLibrary::ModifyNotoriety(Context.WorldContext, Faction, Delta);
}

FText FFVEffect_ModifyNotoriety::GetDescription() const
{
	return FText::Format(LOCTEXT("ModNotoriety", "{0} notoriety {1}"), FVSocialConditions::Name(Faction), FText::AsNumber(Delta));
}

void FFVEffect_ModifyFame::Apply(const FFVConditionContext& Context) const
{
	UFVSocialLibrary::ModifyFame(Context.WorldContext, Delta);
}

FText FFVEffect_ModifyFame::GetDescription() const
{
	return FText::Format(LOCTEXT("ModFame", "Fame {0}"), FText::AsNumber(Delta));
}

void FFVEffect_ModifyGlobalNotoriety::Apply(const FFVConditionContext& Context) const
{
	UFVSocialLibrary::ModifyGlobalNotoriety(Context.WorldContext, Delta);
}

FText FFVEffect_ModifyGlobalNotoriety::GetDescription() const
{
	return FText::Format(LOCTEXT("ModGlobalNotoriety", "Notoriety {0}"), FText::AsNumber(Delta));
}

void FFVEffect_ModifyRelationship::Apply(const FFVConditionContext& Context) const
{
	UFVSocialLibrary::ModifyRelationship(Context.WorldContext, FVSocialConditions::CharacterOrTarget(Character, Context), Delta);
}

FText FFVEffect_ModifyRelationship::GetDescription() const
{
	const FText Who = Character ? Character->Display.Name : LOCTEXT("Target", "target");
	return FText::Format(LOCTEXT("ModRelationship", "Relationship with {0} {1}"), Who, FText::AsNumber(Delta));
}

#undef LOCTEXT_NAMESPACE
