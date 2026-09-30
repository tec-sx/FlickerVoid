#include "Abilities/FVCheck.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCheck)

#define LOCTEXT_NAMESPACE "FVCheck"

FFVCheckResult UFVCheckStatics::RollCheck(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty)
{
return Build(Check, ASC, Difficulty, true);
}

FFVCheckResult UFVCheckStatics::PreviewCheck(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty)
{
return Build(Check, ASC, Difficulty, false);
}

float UFVCheckStatics::SuccessChance(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty)
{
const FFVCheckResult Base = Build(Check, ASC, Difficulty, false);
if (!Check || Check->Roll == EFVCheckRoll::None)
{
return Base.bSuccess ? 1.f : 0.f;
}

const int32 Needed = Difficulty - Base.Total;
if (Check->Roll == EFVCheckRoll::D6x2)
{
int32 Hits = 0;
for (int32 A = 1; A <= 6; ++A)
{
for (int32 B = 1; B <= 6; ++B)
{
Hits += (A + B >= Needed) ? 1 : 0;
}
}
return Hits / 36.f;
}

const int32 Faces = Check->Roll == EFVCheckRoll::D10 ? 10 : 20;
const int32 Passing = FMath::Clamp(Faces - FMath::Max(Needed, 1) + 1, 0, Faces);
return static_cast<float>(Passing) / Faces;
}

UAbilitySystemComponent* UFVCheckStatics::FindASC(const AActor* Actor)
{
return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

FFVCheckResult UFVCheckStatics::Build(const UFVCheckDefinition* Check, const UAbilitySystemComponent* ASC, int32 Difficulty, bool bRoll)
{
FFVCheckResult Result;
Result.Difficulty = Difficulty;
if (!Check)
{
return Result;
}

if (ASC && Check->Attribute.IsValid())
{
bool bFound = false;
const int32 Skill = FMath::RoundToInt(ASC->GetGameplayAttributeValue(Check->Attribute, bFound));
Result.Breakdown.Add({Check->Name, Skill});
}

for (const FFVCheckModifier& Modifier : Check->Modifiers)
{
if (Modifier.RequiredTag.IsValid() && (!ASC || !ASC->HasMatchingGameplayTag(Modifier.RequiredTag)))
{
continue;
}
Result.Breakdown.Add({Modifier.Label, Modifier.Bonus});
}

if (bRoll && Check->Roll != EFVCheckRoll::None)
{
Result.Breakdown.Add({LOCTEXT("Roll", "Roll"), Roll(Check->Roll)});
}

for (const FFVCheckLine& Line : Result.Breakdown)
{
Result.Total += Line.Value;
}
Result.bSuccess = Result.Total >= Difficulty;
return Result;
}

int32 UFVCheckStatics::Roll(EFVCheckRoll InRoll)
{
switch (InRoll)
{
case EFVCheckRoll::D6x2: return FMath::RandRange(1, 6) + FMath::RandRange(1, 6);
case EFVCheckRoll::D10:  return FMath::RandRange(1, 10);
case EFVCheckRoll::D20:  return FMath::RandRange(1, 20);
default:                 return 0;
}
}

bool FFVCondition_PassiveCheck::EvaluateImpl(const FFVConditionContext& Context) const
{
return UFVCheckStatics::PreviewCheck(Check, UFVCheckStatics::FindASC(Cast<AActor>(Context.Instigator)), Difficulty).bSuccess;
}

FText FFVCondition_PassiveCheck::GetDescription() const
{
const FText Name = Check ? Check->Name : LOCTEXT("None", "<no check>");
return FText::Format(LOCTEXT("Passive", "{0} >= {1}"), Name, Difficulty);
}

bool FFVCondition_HasTag::EvaluateImpl(const FFVConditionContext& Context) const
{
const UAbilitySystemComponent* ASC = UFVCheckStatics::FindASC(Cast<AActor>(bCheckTarget ? Context.Target : Context.Instigator));
return ASC && ASC->HasMatchingGameplayTag(Tag);
}

FText FFVCondition_HasTag::GetDescription() const
{
return FText::Format(LOCTEXT("HasTag", "{0} has {1}"),
bCheckTarget ? LOCTEXT("Target", "Target") : LOCTEXT("Instigator", "Instigator"),
FText::FromName(Tag.GetTagName()));
}

#undef LOCTEXT_NAMESPACE