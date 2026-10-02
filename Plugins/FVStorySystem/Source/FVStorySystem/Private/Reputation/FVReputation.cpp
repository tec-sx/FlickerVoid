#include "Reputation/FVReputation.h"

#include "Facts/FVFactDatabase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVReputation)

#define LOCTEXT_NAMESPACE "FVReputation"

const FFVReputationTier* UFVFactionDefinition::FindTier(int32 Standing) const
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

int32 UFVReputationStatics::GetStanding(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
return Database && Faction ? Database->GetFact(Faction->Id) : 0;
}

FGameplayTag UFVReputationStatics::GetTier(const UObject* WorldContext, const UFVFactionDefinition* Faction)
{
if (!Faction)
{
return FGameplayTag();
}
const FFVReputationTier* Tier = Faction->FindTier(GetStanding(WorldContext, Faction));
return Tier ? Tier->Tier : FGameplayTag();
}

void UFVReputationStatics::ModifyStanding(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta)
{
if (!Faction || Delta == 0)
{
return;
}

ApplyDelta(WorldContext, Faction, Delta);
for (const TPair<TObjectPtr<UFVFactionDefinition>, float>& Relation : Faction->Relations)
{
if (Relation.Key && Relation.Key != Faction)
{
ApplyDelta(WorldContext, Relation.Key, FMath::RoundToInt(Delta * Relation.Value));
}
}
}

void UFVReputationStatics::ApplyDelta(UObject* WorldContext, const UFVFactionDefinition* Faction, int32 Delta)
{
UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
if (!Database || Delta == 0)
{
return;
}
const int32 NewValue = FMath::Clamp(Database->GetFact(Faction->Id) + Delta, Faction->MinStanding, Faction->MaxStanding);
Database->SetFact(Faction->Id, NewValue);
}

bool FFVCondition_Standing::EvaluateImpl(const FFVConditionContext& Context) const
{
return Faction && UFVReputationStatics::GetStanding(Context.WorldContext, Faction) >= MinStanding;
}

FText FFVCondition_Standing::GetDescription() const
{
const FText Name = Faction ? Faction->Display.Name : LOCTEXT("None", "<none>");
return FText::Format(LOCTEXT("Standing", "{0} standing >= {1}"), Name, MinStanding);
}

void FFVEffect_ModifyStanding::Apply(const FFVConditionContext& Context) const
{
UFVReputationStatics::ModifyStanding(Context.WorldContext, Faction, Delta);
}

FText FFVEffect_ModifyStanding::GetDescription() const
{
const FText Name = Faction ? Faction->Display.Name : LOCTEXT("None", "<none>");
return FText::Format(LOCTEXT("Modify", "{0} {1}"), Name, FText::AsNumber(Delta));
}

#undef LOCTEXT_NAMESPACE