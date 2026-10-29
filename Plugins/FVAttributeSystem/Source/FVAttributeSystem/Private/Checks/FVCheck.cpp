#include "Checks/FVCheck.h"

#include "Attributes/FVAttributeLibrary.h"
#include "Conditions/FVConditionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCheck)

#define LOCTEXT_NAMESPACE "FVCheck"

namespace
{
	// Chance that the roll comes up Needed or higher; 2d6 counts its 36 outcomes.
	float ChanceAtLeast(EFVCheckRoll Roll, int32 Needed)
	{
		int32 Faces = 0;
		switch (Roll)
		{
		case EFVCheckRoll::D6x2:
			{
				int32 Hits = 0;
				for (int32 First = 1; First <= 6; ++First)
				{
					for (int32 Second = 1; Second <= 6; ++Second)
					{
						Hits += First + Second >= Needed ? 1 : 0;
					}
				}
				return static_cast<float>(Hits) / 36.f;
			}
		case EFVCheckRoll::D10: Faces = 10; break;
		case EFVCheckRoll::D20: Faces = 20; break;
		default: return Needed <= 0 ? 1.f : 0.f;
		}

		return static_cast<float>(FMath::Clamp(Faces - Needed + 1, 0, Faces)) / static_cast<float>(Faces);
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
	return ChanceAtLeast(Check->Roll, Difficulty - Passive.Total);
}

FFVCheckResult UFVCheckLibrary::Build(const UFVCheckDefinition* Check, const FFVConditionContext& Context, int32 Difficulty, bool bRoll)
{
	FFVCheckResult Result;
	Result.Difficulty = Difficulty;

	if (Check == nullptr)
	{
		return Result;
	}

	if (UFVAttributeLibrary::HasAttribute(Context.Instigator, Check->Attribute))
	{
		const int32 Value = FMath::RoundToInt(UFVAttributeLibrary::GetValue(Context.Instigator, Check->Attribute));
		Result.Total += Value;
		Result.Breakdown.Add({ Check->Attribute->Display.Name, Value });
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
