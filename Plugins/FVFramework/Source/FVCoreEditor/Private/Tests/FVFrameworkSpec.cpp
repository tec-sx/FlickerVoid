#include "Conditions/FVCompare.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVConditionLibrary.h"
#include "Conditions/FVEffect.h"
#include "Facts/FVFactConditions.h"
#include "Facts/FVFactDatabase.h"
#include "Facts/FVFactSettings.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"
#include "NativeGameplayTags.h"
#include "Scales/FVScaleDefinition.h"
#include "Scales/FVScaleLibrary.h"
#include "Tests/FVTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FVFrameworkSpec
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Test_A, "Test.FVFramework.A");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Test_B, "Test.FVFramework.B");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Test_Group, "Test.FVFramework.Group");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Test_Group_One, "Test.FVFramework.Group.One");
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Test_Group_Two, "Test.FVFramework.Group.Two");

	struct FCompareCase
	{
		int32 Current;
		EFVCompare Compare;
		int32 Value;
		bool bExpected;
		const TCHAR* What;
	};

	/** An empty group passes, so an inverted one fails: constants without a test-only condition type. */
	void AddConstant(FFVConditionSet& Set, const bool bValue)
	{
		FFVCondition_Group Group;
		Group.bInvert = !bValue;
		Set.Conditions.Add(TInstancedStruct<FFVConditionBase>::Make(Group));
	}

	void AddFactCondition(FFVConditionSet& Set, const FGameplayTag Fact, const EFVFactCompare Compare, const int32 Value)
	{
		FFVCondition_Fact Condition;
		Condition.Fact = Fact;
		Condition.Compare = Compare;
		Condition.Value = Value;
		Set.Conditions.Add(TInstancedStruct<FFVConditionBase>::Make(Condition));
	}

	void AddFactEffect(FFVEffectList& List, const FGameplayTag Fact, const EFVFactWrite Write, const int32 Value)
	{
		FFVEffect_Fact Effect;
		Effect.Fact = Fact;
		Effect.Write = Write;
		Effect.Value = Value;
		List.Effects.Add(TInstancedStruct<FFVEffectBase>::Make(Effect));
	}

	void AddRange(UFVFactSettings& Settings, const FGameplayTag Fact, const int32 Min, const int32 Max)
	{
		FFVFactDefinition& Definition = Settings.Definitions.AddDefaulted_GetRef();
		Definition.Tag = Fact;
		Definition.bClamp = true;
		Definition.Min = Min;
		Definition.Max = Max;
	}
}

BEGIN_DEFINE_SPEC(FFVFrameworkSpec, "FlickerVoid.Framework", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	TUniquePtr<FFVTestWorld> TestWorld;
	UFVFactDatabase* Facts = nullptr;
	FFVConditionContext Context;
	TArray<FFVFactDefinition> SavedDefinitions;

	/** Starts a test world and keeps the fact settings, which tests may add ranges to. */
	void StartWorld();
	void EndWorld();
END_DEFINE_SPEC(FFVFrameworkSpec)

void FFVFrameworkSpec::StartWorld()
{
	TestWorld = MakeUnique<FFVTestWorld>();
	UWorld* World = TestWorld->GetWorld();
	Facts = UFVFactDatabase::Get(World);
	Context = FFVConditionContext();
	Context.WorldContext = World;

	const UFVFactSettings* Settings = GetDefault<UFVFactSettings>();
	SavedDefinitions = Settings->Definitions;
}

void FFVFrameworkSpec::EndWorld()
{
	UFVFactSettings* Settings = GetMutableDefault<UFVFactSettings>();
	Settings->Definitions = SavedDefinitions;
	Facts = nullptr;
	Context = FFVConditionContext();
	TestWorld.Reset();
}

void FFVFrameworkSpec::Define()
{
	using namespace FVFrameworkSpec;

	Describe("Compare", [this]()
	{
		It("compares integers with every operator", [this]()
		{
			const FCompareCase Cases[] = {
				{ 3, EFVCompare::Equal, 3, true, TEXT("3 == 3") },
				{ 3, EFVCompare::NotEqual, 4, true, TEXT("3 != 4") },
				{ 4, EFVCompare::Greater, 3, true, TEXT("4 > 3") },
				{ 3, EFVCompare::Greater, 3, false, TEXT("3 > 3") },
				{ 3, EFVCompare::GreaterOrEqual, 3, true, TEXT("3 >= 3") },
				{ 2, EFVCompare::Less, 3, true, TEXT("2 < 3") },
				{ 3, EFVCompare::Less, 3, false, TEXT("3 < 3") },
				{ 3, EFVCompare::LessOrEqual, 3, true, TEXT("3 <= 3") }
			};

			for (const FCompareCase& Case : Cases)
			{
				const bool bResult = FVCompare::Compare(Case.Current, Case.Compare, Case.Value);
				TestEqual(Case.What, bResult, Case.bExpected);
			}
		});

		It("allows for float error in == and !=", [this]()
		{
			const float Sum = 0.1f + 0.2f;
			const bool bEqual = FVCompare::Compare(Sum, EFVCompare::Equal, 0.3f);
			const bool bNotEqual = FVCompare::Compare(Sum, EFVCompare::NotEqual, 0.3f);
			TestTrue(TEXT("=="), bEqual);
			TestFalse(TEXT("!="), bNotEqual);
		});

		It("scores only at-least comparisons", [this]()
		{
			FFVConditionScore Score;
			const bool bAtLeast = FVCompare::Score(85.f, EFVCompare::GreaterOrEqual, 80.f, Score);
			TestTrue(TEXT(">= is scored"), bAtLeast);
			TestEqual(TEXT("Threshold"), Score.Threshold, 80.f);
			TestEqual(TEXT("Excess"), Score.Excess, 5.f);

			const bool bAtMost = FVCompare::Score(85.f, EFVCompare::LessOrEqual, 80.f, Score);
			const bool bEqual = FVCompare::Score(80.f, EFVCompare::Equal, 80.f, Score);
			TestFalse(TEXT("<= is not scored"), bAtMost);
			TestFalse(TEXT("== is not scored"), bEqual);
		});

		It("ranks a higher threshold above a larger excess", [this]()
		{
			const FFVConditionScore Hard = FFVConditionScore::AtLeast(80.f, 80.f);
			const FFVConditionScore Easy = FFVConditionScore::AtLeast(100.f, 50.f);
			const bool bHardHigher = UFVConditionLibrary::IsScoreHigher(Hard, Easy);
			const bool bEasyHigher = UFVConditionLibrary::IsScoreHigher(Easy, Hard);
			TestTrue(TEXT("Hard is higher"), bHardHigher);
			TestFalse(TEXT("Easy is not higher"), bEasyHigher);
		});
	});

	Describe("Condition set", [this]()
	{
		It("passes when empty, in either mode", [this]()
		{
			FFVConditionSet Set;
			const bool bAll = Set.Evaluate(Context);
			Set.Mode = EFVConditionMode::Any;
			const bool bAny = Set.Evaluate(Context);
			TestTrue(TEXT("All"), bAll);
			TestTrue(TEXT("Any"), bAny);
		});

		It("needs every condition in All mode", [this]()
		{
			FFVConditionSet Set;
			AddConstant(Set, true);
			AddConstant(Set, true);
			const bool bAllTrue = Set.Evaluate(Context);
			AddConstant(Set, false);
			const bool bOneFalse = Set.Evaluate(Context);
			TestTrue(TEXT("All true"), bAllTrue);
			TestFalse(TEXT("One false"), bOneFalse);
		});

		It("needs one condition in Any mode", [this]()
		{
			FFVConditionSet Set;
			Set.Mode = EFVConditionMode::Any;
			AddConstant(Set, false);
			AddConstant(Set, false);
			const bool bAllFalse = Set.Evaluate(Context);
			AddConstant(Set, true);
			const bool bOneTrue = Set.Evaluate(Context);
			TestFalse(TEXT("All false"), bAllFalse);
			TestTrue(TEXT("One true"), bOneTrue);
		});

		It("flips a nested group with Invert", [this]()
		{
			FFVCondition_Group Inner;
			AddConstant(Inner.Set, false);
			Inner.bInvert = true;

			FFVConditionSet Set;
			Set.Conditions.Add(TInstancedStruct<FFVConditionBase>::Make(Inner));
			const bool bResult = Set.Evaluate(Context);
			TestTrue(TEXT("NOT (false)"), bResult);
		});

		It("treats an empty Object condition as passing", [this]()
		{
			const FFVCondition_Object Empty;
			FFVConditionSet Set;
			Set.Conditions.Add(TInstancedStruct<FFVConditionBase>::Make(Empty));
			const bool bResult = Set.Evaluate(Context);
			TestTrue(TEXT("Evaluate"), bResult);
		});
	});

	Describe("Facts", [this]()
	{
		BeforeEach([this]() { StartWorld(); });
		AfterEach([this]() { EndWorld(); });

		It("exists in the test world", [this]()
		{
			TestNotNull(TEXT("Fact database"), Facts);
		});

		It("reads 0 for an undefined fact", [this]()
		{
			TestFalse(TEXT("Defined"), Facts->IsFactDefined(TAG_Test_A));
			TestEqual(TEXT("Value"), Facts->GetFact(TAG_Test_A), 0);
		});

		It("sets, adds and removes", [this]()
		{
			Facts->SetFact(TAG_Test_A, 5);
			TestTrue(TEXT("Defined after set"), Facts->IsFactDefined(TAG_Test_A));
			TestEqual(TEXT("Set"), Facts->GetFact(TAG_Test_A), 5);

			Facts->AddFact(TAG_Test_A, 3);
			TestEqual(TEXT("Add"), Facts->GetFact(TAG_Test_A), 8);

			Facts->AddFact(TAG_Test_B);
			TestEqual(TEXT("Add to undefined starts at 0"), Facts->GetFact(TAG_Test_B), 1);

			const bool bRemoved = Facts->RemoveFact(TAG_Test_A);
			TestTrue(TEXT("Remove"), bRemoved);
			TestFalse(TEXT("Defined after remove"), Facts->IsFactDefined(TAG_Test_A));

			const bool bRemovedTwice = Facts->RemoveFact(TAG_Test_A);
			TestFalse(TEXT("Remove twice"), bRemovedTwice);
		});

		It("removes the facts under a parent tag only", [this]()
		{
			Facts->SetFact(TAG_Test_Group_One, 1);
			Facts->SetFact(TAG_Test_Group_Two, 2);
			Facts->SetFact(TAG_Test_A, 3);

			const int32 Removed = Facts->RemoveFactsUnder(TAG_Test_Group);
			TestEqual(TEXT("Removed"), Removed, 2);
			TestFalse(TEXT("One"), Facts->IsFactDefined(TAG_Test_Group_One));
			TestFalse(TEXT("Two"), Facts->IsFactDefined(TAG_Test_Group_Two));
			TestTrue(TEXT("Other fact kept"), Facts->IsFactDefined(TAG_Test_A));
		});

		It("notifies listeners with the old and new value", [this]()
		{
			Facts->SetFact(TAG_Test_A, 2);

			TArray<TPair<int32, int32>> Changes;
			const FDelegateHandle Handle = Facts->OnFactChangedNative().AddLambda([&Changes](const FGameplayTag Tag, const int32 OldValue, const int32 NewValue)
			{
				if (Tag == TAG_Test_A)
				{
					Changes.Emplace(OldValue, NewValue);
				}
			});

			Facts->AddFact(TAG_Test_A, 3);
			Facts->RemoveFact(TAG_Test_A);
			Facts->OnFactChangedNative().Remove(Handle);

			if (TestEqual(TEXT("Notifications"), Changes.Num(), 2))
			{
				TestEqual(TEXT("Add old"), Changes[0].Key, 2);
				TestEqual(TEXT("Add new"), Changes[0].Value, 5);
				TestEqual(TEXT("Remove old"), Changes[1].Key, 5);
				TestEqual(TEXT("Remove new"), Changes[1].Value, 0);
			}

			const FGameplayTag LastChanged = Facts->GetLastChangedTag();
			TestTrue(TEXT("Last changed"), LastChanged == TAG_Test_A);
		});

		It("clamps a fact with a range in the settings", [this]()
		{
			UFVFactSettings* Settings = GetMutableDefault<UFVFactSettings>();
			AddRange(*Settings, TAG_Test_A, -10, 10);

			Facts->SetFact(TAG_Test_A, 50);
			TestEqual(TEXT("Max"), Facts->GetFact(TAG_Test_A), 10);
			Facts->AddFact(TAG_Test_A, -100);
			TestEqual(TEXT("Min"), Facts->GetFact(TAG_Test_A), -10);
		});

		It("evaluates fact conditions", [this]()
		{
			Facts->SetFact(TAG_Test_A, 4);

			FFVConditionSet Set;
			AddFactCondition(Set, TAG_Test_A, EFVFactCompare::GreaterOrEqual, 4);
			AddFactCondition(Set, TAG_Test_A, EFVFactCompare::Defined, 0);
			AddFactCondition(Set, TAG_Test_B, EFVFactCompare::Undefined, 0);
			const bool bHolds = Set.Evaluate(Context);

			AddFactCondition(Set, TAG_Test_A, EFVFactCompare::Greater, 4);
			const bool bHoldsAbove = Set.Evaluate(Context);

			TestTrue(TEXT("Holds"), bHolds);
			TestFalse(TEXT("> 4 fails"), bHoldsAbove);
		});

		It("ranks the sets that hold, hardest first", [this]()
		{
			Facts->SetFact(TAG_Test_A, 50);

			TArray<FFVConditionSet> Sets;
			Sets.AddDefaulted(4);
			AddFactCondition(Sets[0], TAG_Test_A, EFVFactCompare::GreaterOrEqual, 20);
			AddFactCondition(Sets[1], TAG_Test_A, EFVFactCompare::GreaterOrEqual, 80);
			AddFactCondition(Sets[2], TAG_Test_A, EFVFactCompare::GreaterOrEqual, 40);
			AddConstant(Sets[3], true);

			const TArray<int32> Ranked = UFVConditionLibrary::RankConditionSets(Sets, Context);
			const TArray<int32> Expected = { 2, 0, 3 };
			TestTrue(TEXT("Order is 2, 0, 3"), Ranked == Expected);
		});

		It("scores All sets as the sum and Any sets as the best", [this]()
		{
			Facts->SetFact(TAG_Test_A, 50);
			Facts->SetFact(TAG_Test_B, 30);

			FFVConditionSet Set;
			AddFactCondition(Set, TAG_Test_A, EFVFactCompare::GreaterOrEqual, 40);
			AddFactCondition(Set, TAG_Test_B, EFVFactCompare::GreaterOrEqual, 20);

			FFVConditionScore AllScore;
			const bool bAllScored = Set.GetScore(Context, AllScore);
			TestTrue(TEXT("All scored"), bAllScored);
			TestEqual(TEXT("All threshold"), AllScore.Threshold, 60.f);
			TestEqual(TEXT("All excess"), AllScore.Excess, 20.f);

			Set.Mode = EFVConditionMode::Any;
			FFVConditionScore AnyScore;
			const bool bAnyScored = Set.GetScore(Context, AnyScore);
			TestTrue(TEXT("Any scored"), bAnyScored);
			TestEqual(TEXT("Any threshold"), AnyScore.Threshold, 40.f);
		});

		It("applies fact effects in order", [this]()
		{
			FFVEffectList Effects;
			AddFactEffect(Effects, TAG_Test_A, EFVFactWrite::Set, 3);
			AddFactEffect(Effects, TAG_Test_A, EFVFactWrite::Add, 2);
			AddFactEffect(Effects, TAG_Test_B, EFVFactWrite::Set, 1);
			AddFactEffect(Effects, TAG_Test_B, EFVFactWrite::Remove, 0);
			Effects.Apply(Context);

			TestEqual(TEXT("Set then add"), Facts->GetFact(TAG_Test_A), 5);
			TestFalse(TEXT("Set then remove"), Facts->IsFactDefined(TAG_Test_B));
		});
	});

	Describe("Scales", [this]()
	{
		BeforeEach([this]()
		{
			StartWorld();
			UFVFactSettings* Settings = GetMutableDefault<UFVFactSettings>();
			AddRange(*Settings, TAG_Test_A, -100, 100);
			AddRange(*Settings, TAG_Test_B, 0, 100);
		});

		AfterEach([this]() { EndWorld(); });

		It("returns the change that applied after clamping", [this]()
		{
			UFVScaleDefinition* Scale = NewObject<UFVScaleDefinition>();
			Scale->Id = TAG_Test_A;
			UWorld* World = TestWorld->GetWorld();

			const int32 Within = UFVScaleLibrary::ModifyScale(World, Scale, 30);
			const int32 Clamped = UFVScaleLibrary::ModifyScale(World, Scale, 100);
			const int32 AtEdge = UFVScaleLibrary::ModifyScale(World, Scale, 10);
			const int32 Value = UFVScaleLibrary::GetValue(World, Scale);

			TestEqual(TEXT("Within range"), Within, 30);
			TestEqual(TEXT("Clamped"), Clamped, 70);
			TestEqual(TEXT("At the edge"), AtEdge, 0);
			TestEqual(TEXT("Value"), Value, 100);
		});

		It("passes only the applied change through links", [this]()
		{
			UFVScaleDefinition* Fame = NewObject<UFVScaleDefinition>();
			Fame->Id = TAG_Test_B;

			UFVScaleDefinition* Standing = NewObject<UFVScaleDefinition>();
			Standing->Id = TAG_Test_A;
			FFVScaleLink& Link = Standing->Links.AddDefaulted_GetRef();
			Link.Scale = Fame;
			Link.Factor = 0.5f;
			Link.bMagnitude = true;

			UWorld* World = TestWorld->GetWorld();
			Facts->SetFact(TAG_Test_A, 90);

			UFVScaleLibrary::ModifyScale(World, Standing, 40);
			TestEqual(TEXT("Standing"), Facts->GetFact(TAG_Test_A), 100);
			TestEqual(TEXT("Fame from the applied 10"), Facts->GetFact(TAG_Test_B), 5);

			UFVScaleLibrary::ModifyScale(World, Standing, -20);
			TestEqual(TEXT("A loss still raises fame by magnitude"), Facts->GetFact(TAG_Test_B), 15);
		});

		It("finds the highest band the value reaches", [this]()
		{
			UFVScaleDefinition* Scale = NewObject<UFVScaleDefinition>();
			FFVScaleBand& Low = Scale->Bands.AddDefaulted_GetRef();
			Low.Threshold = 0;
			FFVScaleBand& High = Scale->Bands.AddDefaulted_GetRef();
			High.Threshold = 50;

			const FFVScaleBand* Below = Scale->FindBand(-1);
			const FFVScaleBand* Middle = Scale->FindBand(49);
			const FFVScaleBand* Top = Scale->FindBand(50);
			TestNull(TEXT("Below every band"), Below);
			TestTrue(TEXT("Low band"), Middle != nullptr && Middle->Threshold == 0);
			TestTrue(TEXT("High band"), Top != nullptr && Top->Threshold == 50);
		});
	});

	Describe("Definition validation", [this]()
	{
		It("fails a definition that needs an Id and has none", [this]()
		{
			UFVScaleDefinition* Scale = NewObject<UFVScaleDefinition>();
			FDataValidationContext ValidationContext;
			const EDataValidationResult Result = Scale->IsDataValid(ValidationContext);
			TestTrue(TEXT("Invalid"), Result == EDataValidationResult::Invalid);
		});

		It("fails a scale that links to itself", [this]()
		{
			UFVScaleDefinition* Scale = NewObject<UFVScaleDefinition>();
			Scale->Id = TAG_Test_A;
			FFVScaleLink& Link = Scale->Links.AddDefaulted_GetRef();
			Link.Scale = Scale;

			FDataValidationContext ValidationContext;
			const EDataValidationResult Result = Scale->IsDataValid(ValidationContext);
			TestTrue(TEXT("Invalid"), Result == EDataValidationResult::Invalid);
		});
	});
}

#endif
