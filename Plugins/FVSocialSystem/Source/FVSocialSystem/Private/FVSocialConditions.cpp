#include "FVSocialConditions.h"

#include "FVFactionDefinition.h"
#include "FVSocialLibrary.h"
#include "FVSocialSubsystem.h"
#include "FVTitleDefinition.h"
#include "Identity/FVIdentityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialConditions)

#define LOCTEXT_NAMESPACE "FVSocialConditions"

namespace FVSocialConditions
{
	FText Op(const EFVFactCompare Compare)
	{
		return StaticEnum<EFVFactCompare>()->GetDisplayNameTextByValue(static_cast<int64>(Compare));
	}

	FText Name(const UFVDefinition* Definition)
	{
		return Definition ? Definition->Display.Name : LOCTEXT("None", "<none>");
	}

	AActor* Pick(const FFVConditionContext& Context, const EFVContextActor Which, const UScriptStruct* Requester)
	{
		return Context.GetActor(Which == EFVContextActor::Target, Requester);
	}

	const UFVCharacterDefinition* CharacterOrTarget(const UFVCharacterDefinition* Character, const FFVConditionContext& Context, const UScriptStruct* Requester)
	{
		if (Character)
		{
			return Character;
		}
		const AActor* Target = Context.GetActor(true, Requester);
		return UFVIdentityComponent::GetActorDefinition(Target);
	}

	int32 ReadFame(const FFVConditionContext& Context, const bool bRecognized, const UScriptStruct* Requester)
	{
		if (!bRecognized)
		{
			return UFVSocialLibrary::GetFame(Context.WorldContext);
		}
		const AActor* Instigator = Context.GetActor(false, Requester);
		return UFVSocialLibrary::GetRecognizedFame(Instigator);
	}
}

bool FFVCondition_Standing::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Faction && FVFacts::CompareValue(UFVSocialLibrary::GetStanding(Context.WorldContext, Faction), Compare, Value);
}

bool FFVCondition_Standing::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return Faction && FVFacts::CompareScore(UFVSocialLibrary::GetStanding(Context.WorldContext, Faction), Compare, Value, OutScore);
}

FText FFVCondition_Standing::GetDescription() const
{
	return FText::Format(LOCTEXT("Standing", "{0} standing {1} {2}"), FVSocialConditions::Name(Faction), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Notoriety::EvaluateImpl(const FFVConditionContext& Context) const
{
	return Faction && FVFacts::CompareValue(UFVSocialLibrary::GetNotoriety(Context.WorldContext, Faction), Compare, Value);
}

bool FFVCondition_Notoriety::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return Faction && FVFacts::CompareScore(UFVSocialLibrary::GetNotoriety(Context.WorldContext, Faction), Compare, Value, OutScore);
}

FText FFVCondition_Notoriety::GetDescription() const
{
	return FText::Format(LOCTEXT("Notoriety", "{0} notoriety {1} {2}"), FVSocialConditions::Name(Faction), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Fame::EvaluateImpl(const FFVConditionContext& Context) const
{
	const int32 Fame = FVSocialConditions::ReadFame(Context, bRecognized, StaticStruct());
	return FVFacts::CompareValue(Fame, Compare, Value);
}

bool FFVCondition_Fame::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	const int32 Fame = FVSocialConditions::ReadFame(Context, bRecognized, StaticStruct());
	return FVFacts::CompareScore(Fame, Compare, Value, OutScore);
}

FText FFVCondition_Fame::GetDescription() const
{
	return FText::Format(LOCTEXT("Fame", "Fame {0} {1}"), FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_GlobalNotoriety::EvaluateImpl(const FFVConditionContext& Context) const
{
	return FVFacts::CompareValue(UFVSocialLibrary::GetGlobalNotoriety(Context.WorldContext), Compare, Value);
}

bool FFVCondition_GlobalNotoriety::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	return FVFacts::CompareScore(UFVSocialLibrary::GetGlobalNotoriety(Context.WorldContext), Compare, Value, OutScore);
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
	const UFVCharacterDefinition* Resolved = FVSocialConditions::CharacterOrTarget(Character, Context, StaticStruct());
	return Resolved && FVFacts::CompareValue(UFVSocialLibrary::GetRelationship(Context.WorldContext, Resolved), Compare, Value);
}

bool FFVCondition_Relationship::GetScoreImpl(const FFVConditionContext& Context, FFVConditionScore& OutScore) const
{
	const UFVCharacterDefinition* Resolved = FVSocialConditions::CharacterOrTarget(Character, Context, StaticStruct());
	return Resolved && FVFacts::CompareScore(UFVSocialLibrary::GetRelationship(Context.WorldContext, Resolved), Compare, Value, OutScore);
}

FText FFVCondition_Relationship::GetDescription() const
{
	const FText Who = Character ? Character->Display.Name : LOCTEXT("Target", "target");
	return FText::Format(LOCTEXT("Relationship", "Relationship with {0} {1} {2}"), Who, FVSocialConditions::Op(Compare), Value);
}

bool FFVCondition_Attitude::EvaluateImpl(const FFVConditionContext& Context) const
{
	const AActor* ObserverActor = FVSocialConditions::Pick(Context, Observer, StaticStruct());
	const AActor* Subject = FVSocialConditions::Pick(Context, Observer == EFVContextActor::Target ? EFVContextActor::Instigator : EFVContextActor::Target, StaticStruct());
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
	const AActor* SubjectActor = FVSocialConditions::Pick(Context, Subject, StaticStruct());
	return UFVSocialLibrary::IsDisguisedAs(SubjectActor, Faction);
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
	const UFVCharacterDefinition* Resolved = FVSocialConditions::CharacterOrTarget(Character, Context, StaticStruct());
	UFVSocialLibrary::ModifyRelationship(Context.WorldContext, Resolved, Delta);
}

FText FFVEffect_ModifyRelationship::GetDescription() const
{
	const FText Who = Character ? Character->Display.Name : LOCTEXT("Target", "target");
	return FText::Format(LOCTEXT("ModRelationship", "Relationship with {0} {1}"), Who, FText::AsNumber(Delta));
}

void FFVEffect_PromoteTitle::Apply(const FFVConditionContext& Context) const
{
	if (UFVSocialSubsystem* Social = UFVSocialSubsystem::Get(Context.WorldContext))
	{
		Social->PromoteTitle(Title);
	}
}

FText FFVEffect_PromoteTitle::GetDescription() const
{
	return FText::Format(LOCTEXT("PromoteTitle", "Promote title {0}"), FVSocialConditions::Name(Title));
}

#undef LOCTEXT_NAMESPACE
