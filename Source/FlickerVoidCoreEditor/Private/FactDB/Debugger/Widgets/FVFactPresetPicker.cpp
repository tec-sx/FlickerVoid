// Copyright 2024, Maksym Kapelianovych. Licensed under MIT license.
#include "FVFactPresetPicker.h"

#include "FactDB/Debugger/FVFactDebuggerStyle.h"
#include "FVCore/Public/FactDB/FVFactPreset.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"

SFVFactPresetPicker::SFVFactPresetPicker()
	: CurrentSortMode(EColumnSortMode::Ascending)
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFVFactPresetPicker::Construct(const FArguments& InArgs, const TArray<FAssetData>& PresetsData)
{
	OnPresetSelected = InArgs._OnPresetSelected;

	CachePresetsData(PresetsData);

	ChildSlot
	[
		SNew(SVerticalBox)

		// -------------------------------------------------------------------------------------------------------------
		// Search box
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(4.f, 2.f)
		.AutoHeight()
		[
			SAssignNew(SearchBox, SSearchBox)
			.OnTextChanged(this, &SFVFactPresetPicker::HandleSearchTextChanged)
			.OnTextCommitted(this, &SFVFactPresetPicker::HandleSearchTextCommitted)
			.OnKeyDownHandler(this, &SFVFactPresetPicker::HandleKeyDownFromSearchBox)
		]

		// -------------------------------------------------------------------------------------------------------------
		// Presets list
		+ SVerticalBox::Slot()
		[
			SNew(SBorder)
			.Padding(6.f)
			.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
			[
				SAssignNew(PresetsListView, SListView< TSharedPtr< FAssetData > >)
				.SelectionMode(ESelectionMode::Type::Single)
				.ListItemsSource(&FilteredPresetAssets)
				.OnGenerateRow(this, &SFVFactPresetPicker::HandleGeneratePresetWidget)
				.OnSelectionChanged(this, &SFVFactPresetPicker::HandleSelectionChanged)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("Name")
					.DefaultLabel(NSLOCTEXT("FactDebugger", "ProfilerListColName", "Name"))
					.SortPriority(EColumnSortPriority::Primary)
					.SortMode(this, &SFVFactPresetPicker::GetColumnSortMode)
					.OnSort(this, &SFVFactPresetPicker::HandleSortListView)
				)
			]
		]
	];

	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SFVFactPresetPicker::SetFocusPostConstruct));
	HandleSearchTextChanged(FText::GetEmpty());
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SFVFactPresetPicker::CachePresetsData(const TArray<FAssetData>& PresetsData)
{
	AllPresetAssets.Reserve(PresetsData.Num());
	for (const FAssetData& AssetData : PresetsData)
	{
		AllPresetAssets.Add(MakeShared<FAssetData>(AssetData));
	}

	AllPresetAssets.Sort([](const TSharedPtr<FAssetData>& Lhs, const TSharedPtr<FAssetData>& Rhs)
	{
		return Lhs->AssetName.Compare(Rhs->AssetName) < 0;
	});
}

TSharedRef<ITableRow> SFVFactPresetPicker::HandleGeneratePresetWidget(TSharedPtr<FAssetData> AssetData,
                                                                    const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!ensure(AssetData.IsValid()))
	{
		return SNew(STableRow< TSharedPtr< FAssetData > >, OwnerTable);
	}

	return SNew(STableRow< TSharedPtr< FAssetData > >, OwnerTable)
		.Style(FAppStyle::Get(), "TableView.AlternatingRow")
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f, 0.f, 0.f, 0.f)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
					[
						SNew(SImage)
						.Image(FFVFactDebuggerStyle::Get().GetBrush("ClassThumbnail.FactPreset"))
						.DesiredSizeOverride(FVector2d{36.f})
					]
				]

				+ SOverlay::Slot()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FFVFactDebuggerStyle::Get().GetColor("Colors.FactPreset"))
					.Padding(0.f, 2.f, 0.f, 0.f)
				]

			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(2.f, 0.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 1.f)
				[
					SNew(STextBlock)
					.Text(FText::FromName(AssetData->AssetName))
					.Font(FFVFactDebuggerStyle::Get().GetFontStyle("NameFont"))
					.HighlightText(SearchBox.Get(), &SSearchBox::GetText)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 1.f)
				[
					SNew(STextBlock)
					.Text(FText::FromName(AssetData->PackagePath))
					.Font(FFVFactDebuggerStyle::Get().GetFontStyle("PathFont"))
				]
			]
		];
}

void SFVFactPresetPicker::HandleSelectionChanged(TSharedPtr<FAssetData> AssetData, ESelectInfo::Type Type)
{
	if (Type == ESelectInfo::Type::Direct || Type == ESelectInfo::Type::OnNavigation)
	{
		return;
	}

	if (OnPresetSelected.IsBound())
	{
		OnPresetSelected.Execute(Cast<UFVFactPreset>(AssetData->GetAsset()));
	}
}

void SFVFactPresetPicker::HandleSortListView(EColumnSortPriority::Type SortPriority, const FName& ColumnName,
                                           EColumnSortMode::Type SortMode)
{
	CurrentSortMode = SortMode;

	if (ColumnName == "Name")
	{
		AllPresetAssets.Sort([ SortMode ](const TSharedPtr<FAssetData>& Lhs, const TSharedPtr<FAssetData>& Rhs)
		{
			int32 CompareResult = Lhs->AssetName.Compare(Rhs->AssetName);
			return SortMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
		});

		FilteredPresetAssets.Sort([ SortMode ](const TSharedPtr<FAssetData>& Lhs, const TSharedPtr<FAssetData>& Rhs)
		{
			int32 CompareResult = Lhs->AssetName.Compare(Rhs->AssetName);
			return SortMode == EColumnSortMode::Ascending ? CompareResult < 0 : CompareResult > 0;
		});
	}

	PresetsListView->RequestListRefresh();
}

EColumnSortMode::Type SFVFactPresetPicker::GetColumnSortMode() const
{
	return CurrentSortMode;
}

EActiveTimerReturnType SFVFactPresetPicker::SetFocusPostConstruct(double InCurrentTime, float InDeltaTime) const
{
	if (SearchBox.IsValid())
	{
		FWidgetPath WidgetToFocusPath;
		FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchBox.ToSharedRef(), WidgetToFocusPath);
		FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
		WidgetToFocusPath.GetWindow()->SetWidgetToFocusOnActivate(SearchBox);
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}

void SFVFactPresetPicker::HandleSearchTextChanged(const FText& Text)
{
	ON_SCOPE_EXIT { PresetsListView->RequestListRefresh(); };

	FilteredPresetAssets.Empty();
	if (Text.IsEmpty())
	{
		FilteredPresetAssets = AllPresetAssets;
		return;
	}


	const FString& FilterString = Text.ToString();

	for (const TSharedPtr<FAssetData>& AssetData : AllPresetAssets)
	{
		if (AssetData->AssetName.ToString().Contains(FilterString))
		{
			FilteredPresetAssets.Add(AssetData);
		}
	}
}

void SFVFactPresetPicker::HandleSearchTextCommitted(const FText& Text, ETextCommit::Type Type)
{
	HandleSearchTextChanged(Text);

	if (Type == ETextCommit::Type::OnEnter)
	{
		TArray<TSharedPtr<FAssetData>> SelectionSet = PresetsListView->GetSelectedItems();
		if (SelectionSet.Num() == 0)
		{
			AdjustActiveSelection(1);
			SelectionSet = PresetsListView->GetSelectedItems();
		}

		if (OnPresetSelected.IsBound())
		{
			OnPresetSelected.Execute(Cast<UFVFactPreset>(SelectionSet[0]->GetAsset()));
		}
	}
}

FReply SFVFactPresetPicker::HandleKeyDownFromSearchBox(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
	int32 SelectionDelta = 0;

	if (KeyEvent.GetKey() == EKeys::Up)
	{
		SelectionDelta = -1;
	}
	else if (KeyEvent.GetKey() == EKeys::Down)
	{
		SelectionDelta = +1;
	}

	if (SelectionDelta != 0)
	{
		AdjustActiveSelection(SelectionDelta);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SFVFactPresetPicker::AdjustActiveSelection(int32 SelectionDelta)
{
	TArray<TSharedPtr<FAssetData>> SelectionSet = PresetsListView->GetSelectedItems();
	int32 SelectedSuggestion = INDEX_NONE;

	if (SelectionSet.Num() > 0)
	{
		if (FilteredPresetAssets.Find(SelectionSet[0], /*out*/ SelectedSuggestion) == false)
		{
			// Should never happen
			ensureMsgf(false, TEXT( "SFVFactPresetPicker has a selected item that wasn't in the filtered list" ));
			return;
		}
	}
	else
	{
		SelectedSuggestion = 0;
		SelectionDelta = 0;
	}

	if (FilteredPresetAssets.Num() > 0)
	{
		// Move up or down one, wrapping around
		SelectedSuggestion = (SelectedSuggestion + SelectionDelta + FilteredPresetAssets.Num()) % FilteredPresetAssets.
			Num();

		// Pick the new asset
		const TSharedPtr<FAssetData>& NewSelection = FilteredPresetAssets[SelectedSuggestion];

		PresetsListView->RequestScrollIntoView(NewSelection);
		PresetsListView->SetSelection(NewSelection);
	}
	else
	{
		PresetsListView->ClearSelection();
	}
}
