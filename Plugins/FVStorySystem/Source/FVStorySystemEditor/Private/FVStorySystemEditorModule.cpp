#include "FVCoreEditor.h"
#include "Kismet/GameplayStatics.h"
#include "Modules/ModuleManager.h"
#include "Save/FVSaveSystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "FVSaveInspector"

static const FName SaveInspectorTab("FVSaveInspector");

class SFVSaveInspector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFVSaveInspector)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(4)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SFVSaveInspector::HandleRefresh)
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SNew(SSplitter)
				+ SSplitter::Slot().Value(0.35f)
				[
					SAssignNew(SlotList, SListView<TSharedPtr<FFVSaveMetadata>>)
					.ListItemsSource(&Slots)
					.OnGenerateRow(this, &SFVSaveInspector::MakeSlotRow)
					.OnSelectionChanged(this, &SFVSaveInspector::HandleSelection)
				]
				+ SSplitter::Slot().Value(0.65f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(Details, STextBlock).AutoWrapText(true)
					]
				]
			]
		];
		Refresh();
	}

private:
	FReply HandleRefresh()
	{
		Refresh();
		return FReply::Handled();
	}

	void Refresh()
	{
		Slots.Reset();
		const FString IndexName = GetDefault<UFVSaveSettings>()->SlotIndexName;
		if (const UFVSlotIndex* Index = Cast<UFVSlotIndex>(UGameplayStatics::LoadGameFromSlot(IndexName, 0)))
		{
			for (const FFVSaveMetadata& Meta : Index->Slots)
			{
				Slots.Add(MakeShared<FFVSaveMetadata>(Meta));
			}
		}
		SlotList->RequestListRefresh();
		Details->SetText(FText::GetEmpty());
	}

	TSharedRef<ITableRow> MakeSlotRow(TSharedPtr<FFVSaveMetadata> Item, const TSharedRef<STableViewBase>& Owner)
	{
		const FString Label = FString::Printf(TEXT("%s%s  [%s]"), *Item->SlotName,
		                                      Item->bIsCheckpoint ? TEXT(" (CP)") : TEXT(""),
		                                      *Item->Timestamp.ToString());
		return SNew(STableRow<TSharedPtr<FFVSaveMetadata>>, Owner)
			[
				SNew(STextBlock).Text(FText::FromString(Label))
			];
	}

	void HandleSelection(TSharedPtr<FFVSaveMetadata> Item, ESelectInfo::Type)
	{
		Details->SetText(Item ? FText::FromString(Describe(Item->SlotName)) : FText::GetEmpty());
	}

	static FString Describe(const FString& SlotName)
	{
		const UFVSaveGame* Save = Cast<UFVSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (!Save)
		{
			return TEXT("Failed to load slot.");
		}

		const FFVSaveMetadata& Meta = Save->Metadata;
		FString Out = FString::Printf(
			TEXT(
				"Version: %d\nMap: %s\nPlayTime: %.0fs\nThumbnail: %d bytes\nFlow components: %d\nFlow instances: %d\n\nFacts (%d):\n"),
			Meta.Version, *Meta.MapName, Meta.PlayTimeSeconds, Meta.ThumbnailPNG.Num(), Save->FlowComponents.Num(),
			Save->FlowInstances.Num(), Save->Facts.Num());
		for (const TPair<FGameplayTag, int32>& Fact : Save->Facts)
		{
			Out += FString::Printf(TEXT("  %s = %d\n"), *Fact.Key.ToString(), Fact.Value);
		}
		Out += FString::Printf(TEXT("\nCustom data (%d):\n"), Save->CustomData.Num());
		for (const TPair<FName, FString>& Entry : Save->CustomData)
		{
			Out += FString::Printf(TEXT("  %s = %s\n"), *Entry.Key.ToString(), *Entry.Value);
		}
		return Out;
	}

	TArray<TSharedPtr<FFVSaveMetadata>> Slots;
	TSharedPtr<SListView<TSharedPtr<FFVSaveMetadata>>> SlotList;
	TSharedPtr<STextBlock> Details;
};

class FFVStorySystemEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FFVCoreEditorModule::Get().RegisterDebuggerTab(SaveInspectorTab, LOCTEXT("SaveInspector", "Save Inspector"),
		                                               FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
		                                               {
			                                               return SNew(SDockTab).TabRole(ETabRole::NomadTab)
				                                               [
					                                               SNew(SFVSaveInspector)
				                                               ];
		                                               }));
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("FVCoreEditor"))
		{
			FFVCoreEditorModule::Get().UnregisterDebuggerTab(SaveInspectorTab);
		}
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFVStorySystemEditorModule, FVStorySystemEditor)
