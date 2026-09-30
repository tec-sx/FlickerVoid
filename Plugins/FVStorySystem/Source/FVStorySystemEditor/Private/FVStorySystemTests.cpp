#include "Conditions/FVCondition.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Save/FVSaveSystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVSaveRoundTripTest, "FlickerVoid.Save.RoundTrip",
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVSaveRoundTripTest::RunTest(const FString& Parameters)
{
const FString Slot = TEXT("FV_Test_RoundTrip");
const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Game"), false);

UFVSaveGame* Save = Cast<UFVSaveGame>(UGameplayStatics::CreateSaveGameObject(UFVSaveGame::StaticClass()));
Save->Metadata.Version = UFVSaveGame::CurrentVersion;
Save->Metadata.SlotName = Slot;
Save->CustomData.Add(TEXT("Key"), TEXT("Value"));
if (Tag.IsValid())
{
Save->Facts.Add(Tag, 7);
}

TestTrue(TEXT("Saved"), UGameplayStatics::SaveGameToSlot(Save, Slot, 0));
const UFVSaveGame* Loaded = Cast<UFVSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
UGameplayStatics::DeleteGameInSlot(Slot, 0);

if (!TestNotNull(TEXT("Loaded"), Loaded))
{
return false;
}

TestEqual(TEXT("Version"), Loaded->Metadata.Version, UFVSaveGame::CurrentVersion);
TestEqual(TEXT("Custom data"), Loaded->CustomData.FindRef(TEXT("Key")), FString(TEXT("Value")));
if (Tag.IsValid())
{
TestEqual(TEXT("Fact"), Loaded->Facts.FindRef(Tag), 7);
}
return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFVEmptyConditionSetTest, "FlickerVoid.Conditions.EmptySetPasses",
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFVEmptyConditionSetTest::RunTest(const FString& Parameters)
{
const FFVConditionSet Set;
TestTrue(TEXT("IsEmpty"), Set.IsEmpty());
TestTrue(TEXT("Evaluate"), Set.Evaluate(FFVConditionContext()));
return true;
}

#endif