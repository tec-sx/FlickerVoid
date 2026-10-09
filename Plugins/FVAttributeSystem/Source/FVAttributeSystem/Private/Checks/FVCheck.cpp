#include "Checks/FVCheck.h"

#include "Attributes/FVAttributeComponent.h"
#include "Conditions/FVConditionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCheck)

#define LOCTEXT_NAMESPACE "FVCheck"

namespace
{
	int32 MaxRoll(EFVCheckRoll Roll)
	{
		switch (Roll)
		{
		case EFVCheckRoll::D6x2: return 12;
		case EFVCheckRoll::D10: return 10;
		case EFVCheckRoll::D20: return 20;
		default: return 0;
		}
	}
}

FFVCheckResult UFVCheckLibrary::RollCheck(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty)
{
	return Build(Check, UFVConditionLibrary::MakeContext(Instigator, Target), Difficulty, true);
}

FFVCheckResult UFVCheckLibrary::PreviewCheck(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty)
{
	return Build(Check, UFVConditionLibrary::MakeContext(Instigator, Target), Difficulty, false);
}

float UFVCheckLibrary::SuccessChance(const UFVCheckDefinition* Check, AActor* Instigator, AActor* Target, int32 Difficulty)
{
	if (Check == nullptr)
	{
		return 0.f;
	}

	const FFVCheckResult Passive = PreviewCheck(Check, Instigator, Target, Difficulty);
	const int32 Faces = MaxRoll(Check->Roll);

	if (Faces == 0)
	{
		return Passive.bSuccess ? 1.f : 0.f;
	}

	const int32 Needed = Difficulty - Passive.Total;
	if (Needed <= 1)
	{
		return 1.f;
	}

	return Needed > Faces ? 0.f : static_cast<float>(Faces - Needed + 1) / static_cast<float>(Faces);
}

FFVCheckResult UFVCheckLibrary::Build(const UFVCheckDefinition* Check, const FFVConditionContext& Context, int32 Difficulty, bool bRoll)
{
	FFVCheckResult Result;
	Result.Difficulty = Difficulty;

	if (Check == nullptr)
	{
		return Result;
	}

	if (const UFVAttributeComponent* Attributes = UFVAttributeComponent::Find(Cast<AActor>(Context.Instigator)))
	{
		const int32 Value = FMath::RoundToInt(Attributes->GetValue(Check->Attribute));
		Result.Total += Value;
		Result.Breakdown.Add({ Check->Attribute != nullptr ? Check->Attribute->Display.Name : FText::GetEmpty(), Value });
	}

	for (const FFVCheckModifier& Modifier : Check->Modifiers)
	{
		if (Modifier.Conditions.IsEmpty() || Modifier.Conditions.Evaluate(Context))
		{
			Result.Total += Modifier.Bonus;
			Result.Breakdown.Add({ Modifier.Label, Modifier.Bonus });
		}
	}

	if (bRoll)
	{
		const int32 Rolled = Roll(Check->Roll);
		Result.Total += Rolled;
		Result.Breakdown.Add({ LOCTEXT("Roll", "Roll"), Rolled });
	}

	Result.bSuccess = Result.Total >= Difficulty;
	return Result;
}

int32 UFVCheckLibrary::Roll(EFVCheckRoll InRoll)
{
	switch (InRoll)
	{
	case EFVCheckRoll::D6x2: return FMath::RandRange(1, 6) + FMath::RandRange(1, 6);
	case EFVCheckRoll::D10: return FMath::RandRange(1, 10);
	case EFVCheckRoll::D20: return FMath::RandRange(1, 20);
	default: return 0;
	}
}

FText FFVCondition_PassiveCheck::GetDescription() const
{
	if (Check == nullptr)
	{
		return FText::GetEmpty();
	}

	return FText::Format(LOCTEXT("PassiveDesc", "{0} check ({1})"), Check->Display.Name, FText::AsNumber(Difficulty));
}

bool FFVCondition_PassiveCheck::EvaluateImpl(const FFVConditionContext& Context) const
{
	return UFVCheckLibrary::Build(Check, Context, Difficulty, false).bSuccess;
}

#undef LOCTEXT_NAMESPACE
