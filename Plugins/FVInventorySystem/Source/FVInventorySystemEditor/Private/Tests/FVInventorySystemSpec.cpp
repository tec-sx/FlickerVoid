#include "FVInventorySettings.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVItemDefinition.h"
#include "Misc/AutomationTest.h"
#include "Tests/FVTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace FVInventorySystemSpec
{
	struct FLimits
	{
		bool bWeight = true;
		bool bSpace = true;
		float WeightLimit = 0.f;
		int32 PocketCells = 0;
	};

	UFVItemDefinition* MakeItem(const float Weight, const int32 MaxQuantity, const FIntPoint GridSize = FIntPoint(1, 1))
	{
		UFVItemDefinition* Item = NewObject<UFVItemDefinition>();
		Item->Weight = Weight;
		Item->MaxQuantity = MaxQuantity;
		Item->GridSize = GridSize;
		return Item;
	}
}

BEGIN_DEFINE_SPEC(FFVInventorySystemSpec, "FlickerVoid.InventorySystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	TUniquePtr<FFVTestWorld> TestWorld;
	UFVInventoryComponent* Inventory = nullptr;
	FVInventorySystemSpec::FLimits SavedLimits;

	/** Sets the limits the tests rely on; the project's own values are restored after each test. */
	void SetLimits(bool bWeight, bool bSpace, float WeightLimit, int32 PocketCells) const;
END_DEFINE_SPEC(FFVInventorySystemSpec)

void FFVInventorySystemSpec::SetLimits(const bool bWeight, const bool bSpace, const float WeightLimit, const int32 PocketCells) const
{
	UFVInventorySettings* Settings = GetMutableDefault<UFVInventorySettings>();
	Settings->bEnforceWeight = bWeight;
	Settings->bEnforceSpace = bSpace;
	Settings->BaseWeightLimit = WeightLimit;
	Settings->BasePocketCells = PocketCells;
}

void FFVInventorySystemSpec::Define()
{
	using namespace FVInventorySystemSpec;

	BeforeEach([this]()
	{
		const UFVInventorySettings* Settings = GetDefault<UFVInventorySettings>();
		SavedLimits.bWeight = Settings->bEnforceWeight;
		SavedLimits.bSpace = Settings->bEnforceSpace;
		SavedLimits.WeightLimit = Settings->BaseWeightLimit;
		SavedLimits.PocketCells = Settings->BasePocketCells;

		TestWorld = MakeUnique<FFVTestWorld>();
		AActor* Owner = TestWorld->SpawnActor();
		Inventory = TestWorld->AddComponent<UFVInventoryComponent>(Owner);
	});

	AfterEach([this]()
	{
		SetLimits(SavedLimits.bWeight, SavedLimits.bSpace, SavedLimits.WeightLimit, SavedLimits.PocketCells);
		Inventory = nullptr;
		TestWorld.Reset();
	});

	Describe("Weight", [this]()
	{
		It("adds only the units that fit", [this]()
		{
			SetLimits(true, false, 10.f, 0);
			UFVItemDefinition* Item = MakeItem(3.f, 0);

			const int32 Added = Inventory->AddItem(Item, 5);
			const bool bFitsOneMore = Inventory->CanAddItem(Item, 1);
			TestEqual(TEXT("Added"), Added, 3);
			TestEqual(TEXT("Quantity"), Inventory->GetQuantity(Item), 3);
			TestEqual(TEXT("Total weight"), Inventory->GetTotalWeight(), 9.f);
			TestFalse(TEXT("Fits one more"), bFitsOneMore);
		});

		It("counts the bonus the game layer grants", [this]()
		{
			SetLimits(true, false, 10.f, 0);
			UFVItemDefinition* Item = MakeItem(3.f, 0);
			Inventory->SetBonuses(5.f, 0);

			const int32 Added = Inventory->AddItem(Item, 10);
			TestEqual(TEXT("Added within 15 kg"), Added, 5);
		});

		It("ignores the limit for story items", [this]()
		{
			SetLimits(true, false, 10.f, 0);
			UFVItemDefinition* Item = MakeItem(3.f, 0);

			const int32 Added = Inventory->AddStoryItem(Item, 5);
			TestEqual(TEXT("Added"), Added, 5);
			TestEqual(TEXT("Quantity"), Inventory->GetQuantity(Item), 5);
		});
	});

	Describe("Space", [this]()
	{
		It("gives every new stack its own cells", [this]()
		{
			SetLimits(false, true, 0.f, 2);
			UFVItemDefinition* Item = MakeItem(1.f, 1);

			const int32 Added = Inventory->AddItem(Item, 5);
			TestEqual(TEXT("Added"), Added, 2);
			TestEqual(TEXT("Used cells"), Inventory->GetUsedCells(), 2);
			TestEqual(TEXT("Stacks"), Inventory->GetItems().Num(), 2);
		});

		It("fills an unlimited stack for free", [this]()
		{
			SetLimits(false, true, 0.f, 1);
			UFVItemDefinition* Item = MakeItem(1.f, 0);

			const int32 First = Inventory->AddItem(Item, 100);
			const int32 Second = Inventory->AddItem(Item, 50);
			TestEqual(TEXT("First add"), First, 100);
			TestEqual(TEXT("Second add merges"), Second, 50);
			TestEqual(TEXT("Used cells"), Inventory->GetUsedCells(), 1);
		});

		It("fills existing stacks before claiming new cells", [this]()
		{
			SetLimits(false, true, 0.f, 2);
			UFVItemDefinition* Item = MakeItem(1.f, 10);

			const int32 First = Inventory->AddItem(Item, 4);
			const int32 Second = Inventory->AddItem(Item, 20);
			TestEqual(TEXT("First add"), First, 4);
			TestEqual(TEXT("Second add fills two stacks of 10"), Second, 16);
			TestEqual(TEXT("Quantity"), Inventory->GetQuantity(Item), 20);
		});

		It("refuses an item larger than the free space", [this]()
		{
			SetLimits(false, true, 0.f, 2);
			UFVItemDefinition* Item = MakeItem(1.f, 1, FIntPoint(2, 2));

			const int32 Added = Inventory->AddItem(Item, 1);
			TestEqual(TEXT("Added"), Added, 0);
			TestEqual(TEXT("Stacks"), Inventory->GetItems().Num(), 0);
		});
	});

	Describe("Removing", [this]()
	{
		It("removes nothing when there isn't enough", [this]()
		{
			SetLimits(false, false, 0.f, 0);
			UFVItemDefinition* Item = MakeItem(1.f, 0);
			Inventory->AddItem(Item, 3);

			const bool bTooMany = Inventory->RemoveItem(Item, 4);
			TestFalse(TEXT("Remove 4 of 3"), bTooMany);
			TestEqual(TEXT("Quantity kept"), Inventory->GetQuantity(Item), 3);

			const bool bSome = Inventory->RemoveItem(Item, 2);
			TestTrue(TEXT("Remove 2 of 3"), bSome);
			TestEqual(TEXT("Quantity left"), Inventory->GetQuantity(Item), 1);
		});

		It("spends money only when there is enough", [this]()
		{
			Inventory->AddMoney(10);
			const bool bTooMuch = Inventory->RemoveMoney(11);
			const bool bEnough = Inventory->RemoveMoney(4);
			TestFalse(TEXT("Remove 11 of 10"), bTooMuch);
			TestTrue(TEXT("Remove 4 of 10"), bEnough);
			TestEqual(TEXT("Money left"), Inventory->GetMoney(), 6);
		});
	});
}

#endif
