#include "Checks/FVCheck.h"
#include "Conditions/FVCondition.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FVAttributeSystemSpec
{
	/** Ways to roll at least 2..12 on 2d6, out of 36, worked out by hand rather than by the code under test. */
	const int32 TwoDiceAtLeast[] = { 36, 35, 33, 30, 26, 21, 15, 10, 6, 3, 1 };

	/** A bonus with no conditions, or gated by an inverted empty group, which never passes. */
	void AddModifier(UFVCheckDefinition& Check, const int32 Bonus, const bool bPasses)
	{
		FFVCheckModifier& Modifier = Check.Modifiers.AddDefaulted_GetRef();
		Modifier.Bonus = Bonus;
		if (!bPasses)
		{
			FFVCondition_Group Gate;
			Gate.bInvert = true;
			Modifier.Conditions.Conditions.Add(TInstancedStruct<FFVConditionBase>::Make(Gate));
		}
	}
}

BEGIN_DEFINE_SPEC(FFVAttributeSystemSpec, "FlickerVoid.AttributeSystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FFVAttributeSystemSpec)

void FFVAttributeSystemSpec::Define()
{
	using namespace FVAttributeSystemSpec;

	Describe("Success chance", [this]()
	{
		It("matches the 2d6 table", [this]()
		{
			for (int32 Needed = 2; Needed <= 12; ++Needed)
			{
				const float Chance = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D6x2, Needed);
				const float Expected = static_cast<float>(TwoDiceAtLeast[Needed - 2]) / 36.f;
				const FString What = FString::Printf(TEXT("2d6 >= %d"), Needed);
				TestEqual(*What, Chance, Expected);
			}
		});

		It("is certain below 2 and impossible above 12 on 2d6", [this]()
		{
			const float AtZero = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D6x2, 0);
			const float BelowZero = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D6x2, -5);
			const float AboveMax = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D6x2, 13);
			TestEqual(TEXT("Needed 0"), AtZero, 1.f);
			TestEqual(TEXT("Needed -5"), BelowZero, 1.f);
			TestEqual(TEXT("Needed 13"), AboveMax, 0.f);
		});

		It("counts single dice faces", [this]()
		{
			const float D10Lowest = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D10, 1);
			const float D10Highest = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D10, 10);
			const float D10Above = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D10, 11);
			const float D20Half = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::D20, 11);
			TestEqual(TEXT("d10 >= 1"), D10Lowest, 1.f);
			TestEqual(TEXT("d10 >= 10"), D10Highest, 0.1f);
			TestEqual(TEXT("d10 >= 11"), D10Above, 0.f);
			TestEqual(TEXT("d20 >= 11"), D20Half, 0.5f);
		});

		It("needs nothing to roll without dice", [this]()
		{
			const float NothingNeeded = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::None, 0);
			const float OneNeeded = UFVCheckLibrary::ChanceAtLeast(EFVCheckRoll::None, 1);
			TestEqual(TEXT("Needed 0"), NothingNeeded, 1.f);
			TestEqual(TEXT("Needed 1"), OneNeeded, 0.f);
		});
	});

	Describe("Modifiers", [this]()
	{
		It("adds the bonuses whose conditions pass", [this]()
		{
			UFVCheckDefinition* Check = NewObject<UFVCheckDefinition>();
			Check->Roll = EFVCheckRoll::None;
			AddModifier(*Check, 2, true);
			AddModifier(*Check, 3, true);
			AddModifier(*Check, 100, false);

			const FFVConditionContext Context;
			const FFVCheckResult Result = UFVCheckLibrary::Build(Check, Context, 5, false);
			TestEqual(TEXT("Total"), Result.Total, 5);
			TestEqual(TEXT("Breakdown lines"), Result.Breakdown.Num(), 2);
			TestTrue(TEXT("Meets difficulty 5"), Result.bSuccess);

			const FFVCheckResult Harder = UFVCheckLibrary::Build(Check, Context, 6, false);
			TestFalse(TEXT("Misses difficulty 6"), Harder.bSuccess);
		});

		It("adds the roll to the breakdown when rolling", [this]()
		{
			UFVCheckDefinition* Check = NewObject<UFVCheckDefinition>();
			Check->Roll = EFVCheckRoll::D6x2;

			const FFVConditionContext Context;
			const FFVCheckResult Result = UFVCheckLibrary::Build(Check, Context, 0, true);
			TestTrue(TEXT("Roll in 2..12"), Result.Total >= 2 && Result.Total <= 12);
			TestEqual(TEXT("Breakdown lines"), Result.Breakdown.Num(), 1);
		});
	});
}

#endif
