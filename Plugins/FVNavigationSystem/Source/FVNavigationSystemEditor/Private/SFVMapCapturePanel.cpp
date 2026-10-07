#include "SFVMapCapturePanel.h"

#include "Editor.h"
#include "FVMapCapture.h"
#include "FVMapCaptureActor.h"
#include "FVMapDefinition.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "FVMapCapturePanel"

void SFVMapCapturePanel::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsView = PropertyEditor.CreateDetailView(DetailsArgs);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SFVMapCapturePanel::HandleRefreshClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("CaptureAll", "Capture All"))
				.ToolTipText(LOCTEXT("CaptureAllTip", "Capture every map capture actor in the level."))
				.OnClicked(this, &SFVMapCapturePanel::HandleCaptureAllClicked)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("Hint", "Place a map capture actor (or your Blueprint of it), fit its box over the area and pick the map and layer to fill."))
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot().Value(0.4f)
			[
				SAssignNew(ListView, SListView<TSharedPtr<FItem>>)
				.ListItemsSource(&Items)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SFVMapCapturePanel::MakeRow)
				.OnSelectionChanged(this, &SFVMapCapturePanel::HandleSelectionChanged)
			]
			+ SSplitter::Slot().Value(0.6f)
			[
				DetailsView.ToSharedRef()
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text_Lambda([this] { return Status; })
		]
	];

	if (GEngine)
	{
		GEngine->OnLevelActorAdded().AddSP(this, &SFVMapCapturePanel::HandleActorAdded);
		GEngine->OnLevelActorDeleted().AddSP(this, &SFVMapCapturePanel::HandleActorDeleted);
	}
	FEditorDelegates::MapChange.AddSP(this, &SFVMapCapturePanel::HandleMapChanged);

	Refresh();
}

SFVMapCapturePanel::~SFVMapCapturePanel()
{
	if (GEngine)
	{
		GEngine->OnLevelActorAdded().RemoveAll(this);
		GEngine->OnLevelActorDeleted().RemoveAll(this);
	}
	FEditorDelegates::MapChange.RemoveAll(this);
}

void SFVMapCapturePanel::Refresh()
{
	Items.Reset();
	for (AFVMapCaptureActor* Actor : FVMapCapture::FindCaptureActors())
	{
		Items.Add(MakeShared<FItem>(FItem{Actor}));
	}

	DetailsView->SetObject(nullptr);
	ListView->RequestListRefresh();
}

void SFVMapCapturePanel::Capture(AFVMapCaptureActor* Actor)
{
	const FVMapCapture::FResult Result = FVMapCapture::CaptureAndNotify(Actor);
	Status = Result.Message;
	DetailsView->ForceRefresh();
	ListView->RequestListRefresh();
}

TSharedRef<ITableRow> SFVMapCapturePanel::MakeRow(TSharedPtr<FItem> Item, const TSharedRef<STableViewBase>& Owner)
{
	const TWeakObjectPtr<AFVMapCaptureActor> WeakActor = Item->Actor;

	return SNew(STableRow<TSharedPtr<FItem>>, Owner)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([WeakActor]
					{
						return WeakActor.IsValid() ? FText::FromString(WeakActor->GetActorLabel()) : FText::GetEmpty();
					})
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.Text_Lambda([WeakActor]
					{
						const AFVMapCaptureActor* Actor = WeakActor.Get();
						if (Actor == nullptr)
						{
							return FText::GetEmpty();
						}

						const FIntPoint Size = Actor->GetCaptureSize();
						return FText::Format(LOCTEXT("RowInfo", "{0} / {1}  ({2}x{3})"),
							Actor->Map ? FText::FromString(Actor->Map->GetName()) : LOCTEXT("NoMap", "no map"),
							FText::FromName(Actor->Layer), Size.X, Size.Y);
					})
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Focus", "Focus"))
				.ToolTipText(LOCTEXT("FocusTip", "Select the actor and move the viewport to it."))
				.OnClicked_Lambda([WeakActor]
				{
					if (AFVMapCaptureActor* Actor = WeakActor.Get(); Actor && GEditor)
					{
						GEditor->SelectNone(false, true);
						GEditor->SelectActor(Actor, true, true);
						GEditor->MoveViewportCamerasToActor(*Actor, false);
					}
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("Capture", "Capture"))
				.OnClicked_Lambda([this, WeakActor]
				{
					Capture(WeakActor.Get());
					return FReply::Handled();
				})
			]
		];
}

void SFVMapCapturePanel::HandleSelectionChanged(TSharedPtr<FItem> Item, ESelectInfo::Type SelectInfo)
{
	DetailsView->SetObject(Item.IsValid() ? Item->Actor.Get() : nullptr);
}

void SFVMapCapturePanel::HandleActorAdded(AActor* Actor)
{
	if (Cast<AFVMapCaptureActor>(Actor) != nullptr)
	{
		Refresh();
	}
}

void SFVMapCapturePanel::HandleActorDeleted(AActor* Actor)
{
	if (Cast<AFVMapCaptureActor>(Actor) == nullptr)
	{
		return;
	}

	Items.RemoveAll([Actor](const TSharedPtr<FItem>& Item) { return Item->Actor.Get() == Actor; });
	DetailsView->SetObject(nullptr);
	ListView->RequestListRefresh();
}

void SFVMapCapturePanel::HandleMapChanged(uint32 MapChangeFlags)
{
	Refresh();
}

FReply SFVMapCapturePanel::HandleRefreshClicked()
{
	Refresh();
	return FReply::Handled();
}

FReply SFVMapCapturePanel::HandleCaptureAllClicked()
{
	int32 Succeeded = 0;
	for (const TSharedPtr<FItem>& Item : Items)
	{
		if (AFVMapCaptureActor* Actor = Item->Actor.Get())
		{
			Succeeded += FVMapCapture::CaptureAndNotify(Actor).bSuccess ? 1 : 0;
		}
	}

	Status = FText::Format(LOCTEXT("CapturedAll", "Captured {0} of {1}."), Succeeded, Items.Num());
	DetailsView->ForceRefresh();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
