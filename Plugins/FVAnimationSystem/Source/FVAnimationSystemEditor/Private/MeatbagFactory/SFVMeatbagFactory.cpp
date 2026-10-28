#include "MeatbagFactory/SFVMeatbagFactory.h"

#include "AnimPreviewInstance.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/SkeletalMesh.h"
#include "FVAnimationSettings.h"
#include "FVMirrorNameRule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IPersonaToolkit.h"
#include "PoseAuthoring/FVPoseMirror.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SFVMeatbagFactory"

namespace
{
	TSharedRef<SWidget> Section(const FText& Title, const TSharedRef<SWidget>& Body)
	{
		return SNew(SExpandableArea)
			.AreaTitle(Title)
			.InitiallyCollapsed(false)
			.Padding(FMargin(8.f, 4.f))
			.BodyContent()[Body];
	}

	TSharedRef<SWidget> Labelled(const FText& Label, const TSharedRef<SWidget>& Content)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(SBox).WidthOverride(90.f)[SNew(STextBlock).Text(Label)]
			]
			+ SHorizontalBox::Slot().FillWidth(1.f)[Content];
	}

	FText SideLabel(EFVBodySide Side)
	{
		switch (Side)
		{
		case EFVBodySide::Left: return LOCTEXT("SideLeft", "L");
		case EFVBodySide::Right: return LOCTEXT("SideRight", "R");
		default: return LOCTEXT("SideCentre", "C");
		}
	}

	FText AxisText(EFVMirrorAxis Axis)
	{
		switch (Axis)
		{
		case EFVMirrorAxis::X: return LOCTEXT("AxisX", "X");
		case EFVMirrorAxis::Y: return LOCTEXT("AxisY", "Y");
		case EFVMirrorAxis::Z: return LOCTEXT("AxisZ", "Z");
		default:
			{
				const EAxis::Type Default = UFVAnimationSettings::Get().DefaultMirrorAxis;
				const FText Name = Default == EAxis::Y ? LOCTEXT("AxisY", "Y") : Default == EAxis::Z ? LOCTEXT("AxisZ", "Z") : LOCTEXT("AxisX", "X");
				return FText::Format(LOCTEXT("AxisDefault", "Project default ({0})"), Name);
			}
		}
	}
}

void SFVMeatbagFactory::Construct(const FArguments& InArgs)
{
	PersonaToolkit = InArgs._PersonaToolkit;

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()[Section(LOCTEXT("SetupSection", "Setup"), BuildSetup())]
		+ SScrollBox::Slot()[Section(LOCTEXT("PosesSection", "Poses"), BuildPoses())]
		+ SScrollBox::Slot()[Section(LOCTEXT("ActionsSection", "Actions"), BuildActions())]
	];
}

SFVMeatbagFactory::~SFVMeatbagFactory()
{
	UDebugSkelMeshComponent* Component = GetPreviewComponent();
	UAnimPreviewInstance* Instance = GetPreviewInstance();
	if (Component && Instance && GetPoseAsset() && Instance->GetCurrentAsset() == GetPoseAsset())
	{
		Instance->ResetModifiedBone();
		Component->EnablePreview(true, nullptr);
	}
}

void SFVMeatbagFactory::PostUndo(bool bSuccess)
{
	Refresh();
	ApplyPreview(true);
}

void SFVMeatbagFactory::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

TSharedRef<SWidget> SFVMeatbagFactory::BuildSetup()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
		[
			Labelled(LOCTEXT("PoseAssetLabel", "Pose Asset"),
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(UPoseAsset::StaticClass())
				.ObjectPath(this, &SFVMeatbagFactory::GetPoseAssetPath)
				.OnObjectChanged(this, &SFVMeatbagFactory::OnPoseAssetChanged)
				.OnShouldFilterAsset(this, &SFVMeatbagFactory::ShouldFilterAsset)
				.AllowClear(true)
				.DisplayThumbnail(false)
				.ToolTipText(LOCTEXT("PoseAssetTip", "A Pose Asset on this mesh's skeleton. Create one with Create Asset > Pose Asset in the toolbar.")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
		[
			Labelled(LOCTEXT("MirrorAxisLabel", "Mirror Axis"),
				SNew(SComboButton)
				.ToolTipText(LOCTEXT("MirrorAxisTip", "Override for this session, for creatures or robots whose pairs mirror on another axis. The project default is in Project Settings > FlickerVoid > Animation System."))
				.OnGetMenuContent(this, &SFVMeatbagFactory::MakeAxisMenu)
				.ButtonContent()[SNew(STextBlock).Text(this, &SFVMeatbagFactory::GetAxisText)])
		];
}

TSharedRef<SWidget> SFVMeatbagFactory::BuildPoses()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("FilterHint", "Filter poses"))
			.OnTextChanged_Lambda([this](const FText& Text) { Filter = Text.ToString(); Refresh(); })
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f)
		[
			SNew(SBox).MinDesiredHeight(120.f).MaxDesiredHeight(320.f)
			[
				SAssignNew(PoseList, SListView<FPoseItemPtr>)
				.ListItemsSource(&Items)
				.SelectionMode(ESelectionMode::Multi)
				.OnGenerateRow(this, &SFVMeatbagFactory::MakePoseRow)
				.OnSelectionChanged_Lambda([this](FPoseItemPtr, ESelectInfo::Type) { RefreshCurves(); })
				.OnMouseButtonDoubleClick_Lambda([this](FPoseItemPtr Item) { if (Item.IsValid()) { GoTo(Item->Name); } })
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("CurvesLabel", "Curves of the selected pose"))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SComboButton)
				.IsEnabled(this, &SFVMeatbagFactory::HasAsset)
				.ToolTipText(LOCTEXT("AddCurveTip", "Add a skeleton curve (authored in content, Skeleton > Anim Curves) to the Pose Asset."))
				.OnGetMenuContent(this, &SFVMeatbagFactory::MakeAddCurveMenu)
				.ButtonContent()[SNew(STextBlock).Text(LOCTEXT("AddCurve", "Add Curve"))]
			]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox).MaxDesiredHeight(160.f)
			[
				SAssignNew(CurveList, SListView<FCurveItemPtr>)
				.ListItemsSource(&CurveItems)
				.SelectionMode(ESelectionMode::None)
				.OnGenerateRow(this, &SFVMeatbagFactory::MakeCurveRow)
			]
		];
}

TSharedRef<SWidget> SFVMeatbagFactory::BuildActions()
{
	auto Button = [this](const FText& Label, const FText& ToolTip, FReply (SFVMeatbagFactory::*Handler)(), bool (SFVMeatbagFactory::*Enabled)() const) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.HAlign(HAlign_Center)
			.Text(Label)
			.ToolTipText(ToolTip)
			.IsEnabled(this, Enabled)
			.OnClicked(this, Handler);
	};
	auto Menu = [this](const FText& Label, const FText& ToolTip, TSharedRef<SWidget> (SFVMeatbagFactory::*MakeMenu)(), bool (SFVMeatbagFactory::*Enabled)() const) -> TSharedRef<SWidget>
	{
		return SNew(SComboButton)
			.HAlign(HAlign_Center)
			.ToolTipText(ToolTip)
			.IsEnabled(this, Enabled)
			.OnGetMenuContent(this, MakeMenu)
			.ButtonContent()[SNew(STextBlock).Text(Label)];
	};

	return SNew(SUniformGridPanel)
		.SlotPadding(FMargin(2.f))
		+ SUniformGridPanel::Slot(0, 0)[Menu(LOCTEXT("Create", "Create"), LOCTEXT("CreateTip", "Save the viewport as a new pose: the pose last opened with Go To (or the base pose) plus the bones you edited."), &SFVMeatbagFactory::MakeCreateMenu, &SFVMeatbagFactory::HasAsset)]
		+ SUniformGridPanel::Slot(1, 0)[Button(LOCTEXT("Update", "Update"), LOCTEXT("UpdateTip", "Replace the selected pose with the viewport: the pose last opened with Go To plus the bones you edited. Select one pose."), &SFVMeatbagFactory::UpdatePose, &SFVMeatbagFactory::HasSinglePose)]
		+ SUniformGridPanel::Slot(2, 0)[Button(LOCTEXT("Delete", "Delete"), LOCTEXT("DeleteTip", "Delete the selected poses."), &SFVMeatbagFactory::DeletePoses, &SFVMeatbagFactory::HasSelection)]
		+ SUniformGridPanel::Slot(0, 1)[Button(LOCTEXT("GoTo", "Go To"), LOCTEXT("GoToTip", "Show the selected pose in the viewport and edit from it (double-click a pose does the same). Select one pose."), &SFVMeatbagFactory::GoToSelected, &SFVMeatbagFactory::HasSinglePose)]
		+ SUniformGridPanel::Slot(1, 1)[Button(LOCTEXT("Reset", "Reset"), LOCTEXT("ResetTip", "Back to the base pose: clears preview weights and bone edits."), &SFVMeatbagFactory::ResetPreview, &SFVMeatbagFactory::HasAsset)]
		+ SUniformGridPanel::Slot(2, 1)[Button(LOCTEXT("Duplicate", "Duplicate"), LOCTEXT("DuplicateTip", "Copy the selected pose under a new name. Select one pose."), &SFVMeatbagFactory::DuplicatePose, &SFVMeatbagFactory::HasSinglePose)]
		+ SUniformGridPanel::Slot(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("Mirror", "Mirror"))
				.ToolTipText(this, &SFVMeatbagFactory::GetMirrorToolTip)
				.IsEnabled(this, &SFVMeatbagFactory::CanMirror)
				.OnClicked(this, &SFVMeatbagFactory::MirrorSelected)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SComboButton)
				.ToolTipText(LOCTEXT("MirrorAllTip", "Mirror every pose of one side."))
				.IsEnabled(this, &SFVMeatbagFactory::HasAsset)
				.OnGetMenuContent(this, &SFVMeatbagFactory::MakeMirrorAllMenu)
			]
		]
		+ SUniformGridPanel::Slot(1, 2)[Menu(LOCTEXT("Inbetweens", "Inbetweens"), LOCTEXT("InbetweensTip", "Add poses between the base pose and the selected pose. Select one pose."), &SFVMeatbagFactory::MakeInbetweensMenu, &SFVMeatbagFactory::HasSinglePose)]
		+ SUniformGridPanel::Slot(2, 2)[Menu(LOCTEXT("CopyPaste", "Copy/Paste"), LOCTEXT("CopyPasteTip", "Copy a pose, or only the bones selected in the skeleton tree, and paste it onto the selected poses with a multiplier."), &SFVMeatbagFactory::MakeClipboardMenu, &SFVMeatbagFactory::HasAsset)];
}

TSharedRef<ITableRow> SFVMeatbagFactory::MakePoseRow(FPoseItemPtr Item, const TSharedRef<STableViewBase>& Owner)
{
	const FName Pose = Item->Name;
	TSharedRef<STableRow<FPoseItemPtr>> Row = SNew(STableRow<FPoseItemPtr>, Owner);
	Row->SetContent(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 0.f)
		[
			SNew(SBox).WidthOverride(16.f)
			[
				SNew(STextBlock)
				.Text(SideLabel(Item->Side))
				.ToolTipText(LOCTEXT("SideTip", "Side from the pose name: L, R or C (centre, never mirrored)."))
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(SInlineEditableTextBlock)
			.Text(FText::FromName(Pose))
			.IsSelected(Row, &STableRow<FPoseItemPtr>::IsSelectedExclusively)
			.OnTextCommitted(this, &SFVMeatbagFactory::RenamePose, Pose)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.f, 1.f)
		[
			SNew(SBox).WidthOverride(80.f)
			[
				SNew(SSpinBox<float>)
				.MinValue(0.f)
				.MaxValue(1.f)
				.ToolTipText(LOCTEXT("WeightTip", "Preview weight; only changes the viewport. Create and Update use the pose last opened with Go To."))
				.Value_Lambda([this, Pose] { return Weights.FindRef(Pose); })
				.OnValueChanged_Lambda([this, Pose](float Value) { Weights.Add(Pose, Value); ApplyPreview(false); })
			]
		]);
	return Row;
}

TSharedRef<ITableRow> SFVMeatbagFactory::MakeCurveRow(FCurveItemPtr Curve, const TSharedRef<STableViewBase>& Owner)
{
	const FName Name = *Curve;
	return SNew(STableRow<FCurveItemPtr>, Owner)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(4.f, 0.f)
		[
			SNew(STextBlock).Text(FText::FromName(Name))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 1.f)
		[
			SNew(SBox).WidthOverride(80.f)
			[
				SNew(SSpinBox<float>)
				.MinSliderValue(0.f)
				.MaxSliderValue(1.f)
				.IsEnabled(this, &SFVMeatbagFactory::HasSinglePose)
				.Value_Lambda([this, Name] { return CurveValues.FindRef(Name); })
				.OnValueCommitted(this, &SFVMeatbagFactory::SetCurveValue, Name)
			]
		]
	];
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeAxisMenu()
{
	FMenuBuilder Builder(true, nullptr);
	for (const EFVMirrorAxis Axis : { EFVMirrorAxis::Default, EFVMirrorAxis::X, EFVMirrorAxis::Y, EFVMirrorAxis::Z })
	{
		Builder.AddMenuEntry(AxisText(Axis), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, Axis] { MirrorAxis = Axis; }), FCanExecuteAction(),
				FIsActionChecked::CreateLambda([this, Axis] { return MirrorAxis == Axis; })),
			NAME_None, EUserInterfaceActionType::RadioButton);
	}
	return Builder.MakeWidget();
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeCreateMenu()
{
	if (UPoseAsset* Asset = GetPoseAsset())
	{
		const TArray<FName> Bones = GetSelectedBones();
		NewPoseName = FFVPoseAuthoring::MakeUniquePoseName(*Asset, Bones.IsEmpty() ? TEXT("Pose") : Bones[0].ToString() + TEXT("_pose")).ToString();
	}

	return SNew(SBox).Padding(8.f).MinDesiredWidth(260.f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SEditableTextBox)
			.Text_Lambda([this] { return FText::FromString(NewPoseName); })
			.ToolTipText(LOCTEXT("NewNameTip", "Put the side in the name (upperarm_l_up_90) so the pose can be mirrored."))
			.OnTextChanged_Lambda([this](const FText& Text) { NewPoseName = Text.ToString(); })
			.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type Commit)
			{
				NewPoseName = Text.ToString();
				if (Commit == ETextCommit::OnEnter)
				{
					CreatePose();
				}
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f).HAlign(HAlign_Right)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreatePose", "Create Pose"))
			.OnClicked_Lambda([this] { CreatePose(); return FReply::Handled(); })
		]
	];
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeMirrorAllMenu()
{
	FMenuBuilder Builder(true, nullptr);
	Builder.AddMenuEntry(LOCTEXT("MirrorAllLeft", "Mirror all left poses to the right"), FText::GetEmpty(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { MirrorAll(EFVBodySide::Left); })));
	Builder.AddMenuEntry(LOCTEXT("MirrorAllRight", "Mirror all right poses to the left"), FText::GetEmpty(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { MirrorAll(EFVBodySide::Right); })));
	return Builder.MakeWidget();
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeInbetweensMenu()
{
	return SNew(SBox).Padding(8.f).MinDesiredWidth(200.f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			Labelled(LOCTEXT("InbetweenCount", "Count"),
				SNew(SSpinBox<int32>)
				.MinValue(1)
				.MaxValue(9)
				.Value_Lambda([this] { return InbetweenCount; })
				.OnValueChanged_Lambda([this](int32 Value) { InbetweenCount = Value; }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f).HAlign(HAlign_Right)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateInbetweens", "Create Inbetweens"))
			.OnClicked_Lambda([this] { CreateInbetweens(); return FReply::Handled(); })
		]
	];
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeClipboardMenu()
{
	FMenuBuilder Builder(true, nullptr);
	Builder.AddMenuEntry(LOCTEXT("CopyPose", "Copy pose"), LOCTEXT("CopyPoseTip", "Every bone and curve of the selected pose."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { Copy(false); }), FCanExecuteAction::CreateSP(this, &SFVMeatbagFactory::HasSinglePose)));
	Builder.AddMenuEntry(LOCTEXT("CopyBones", "Copy selected bones"), LOCTEXT("CopyBonesTip", "Only the bones selected in the skeleton tree; no curves."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { Copy(true); }), FCanExecuteAction::CreateSP(this, &SFVMeatbagFactory::HasSinglePose)));
	Builder.AddSeparator();
	Builder.AddWidget(
		SNew(SBox).WidthOverride(120.f)
		[
			SNew(SSpinBox<float>)
			.MinSliderValue(0.f)
			.MaxSliderValue(2.f)
			.ToolTipText(LOCTEXT("MultiplierTip", "1 pastes as copied, 0.5 halves the change from the base pose, 1.5 exaggerates it."))
			.Value_Lambda([this] { return PasteMultiplier; })
			.OnValueChanged_Lambda([this](float Value) { PasteMultiplier = Value; })
		],
		LOCTEXT("Multiplier", "Multiplier"));
	Builder.AddMenuEntry(LOCTEXT("Paste", "Paste onto selected poses"), FText::GetEmpty(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SFVMeatbagFactory::Paste),
			FCanExecuteAction::CreateLambda([this] { return Clipboard.IsSet() && HasSelection(); })));
	return Builder.MakeWidget();
}

TSharedRef<SWidget> SFVMeatbagFactory::MakeAddCurveMenu()
{
	FMenuBuilder Builder(true, nullptr);
	const UPoseAsset* Asset = GetPoseAsset();
	const TArray<FName> Curves = Asset ? FFVPoseAuthoring::GetAddableCurves(*Asset) : TArray<FName>();
	if (Curves.IsEmpty())
	{
		Builder.AddWidget(SNew(STextBlock).Text(LOCTEXT("NoCurves", "No skeleton curves to add. Author them on the Skeleton (Anim Curves).")), FText::GetEmpty());
	}
	for (const FName Curve : Curves)
	{
		Builder.AddMenuEntry(FText::FromName(Curve), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SFVMeatbagFactory::AddCurve, Curve)));
	}
	return Builder.MakeWidget();
}

UPoseAsset* SFVMeatbagFactory::GetPoseAsset() const
{
	return PoseAsset.Get();
}

FString SFVMeatbagFactory::GetPoseAssetPath() const
{
	return PoseAsset.IsValid() ? PoseAsset->GetPathName() : FString();
}

void SFVMeatbagFactory::OnPoseAssetChanged(const FAssetData& AssetData)
{
	if (UAnimPreviewInstance* Instance = GetPreviewInstance())
	{
		Instance->ResetModifiedBone();
	}
	Weights.Reset();
	ActivePose = NAME_None;
	ApplyPreview(false);

	PoseAsset = Cast<UPoseAsset>(AssetData.GetAsset());
	Refresh();
}

bool SFVMeatbagFactory::ShouldFilterAsset(const FAssetData& AssetData) const
{
	const TSharedPtr<IPersonaToolkit> Toolkit = PersonaToolkit.Pin();
	const USkeleton* Skeleton = Toolkit.IsValid() ? Toolkit->GetSkeleton() : nullptr;
	return Skeleton && Skeleton->ShouldFilterAsset(AssetData);
}

FText SFVMeatbagFactory::GetAxisText() const
{
	return AxisText(MirrorAxis);
}

void SFVMeatbagFactory::Refresh()
{
	const TArray<FName> Selected = GetSelectedPoses();
	Items.Reset();
	if (const UPoseAsset* Asset = GetPoseAsset())
	{
		const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
		for (const FName Pose : Asset->GetPoseFNames())
		{
			if (Filter.IsEmpty() || Pose.ToString().Contains(Filter))
			{
				Items.Add(MakeShared<FPoseItem>(FPoseItem{ Pose, Rule.GetNameSide(Pose.ToString()) }));
			}
		}
	}
	if (PoseList.IsValid())
	{
		PoseList->RequestListRefresh();
	}
	SelectPoses(Selected);
	RefreshCurves();
}

void SFVMeatbagFactory::RefreshCurves()
{
	CurveItems.Reset();
	CurveValues.Reset();
	if (const UPoseAsset* Asset = GetPoseAsset())
	{
		FFVPoseSnapshot Snapshot;
		const bool bHasPose = HasSinglePose() && FFVPoseAuthoring::ReadPose(*Asset, GetSinglePose(), Snapshot);
		for (const FName Curve : Asset->GetCurveFNames())
		{
			CurveItems.Add(MakeShared<FName>(Curve));
			CurveValues.Add(Curve, bHasPose ? Snapshot.Curves.FindRef(Curve) : 0.f);
		}
	}
	if (CurveList.IsValid())
	{
		CurveList->RequestListRefresh();
	}

	// Checked here, on selection and edits, rather than every frame by the Mirror button.
	bCanMirror = false;
	MirrorReason = FText::GetEmpty();
	if (const UPoseAsset* Asset = GetPoseAsset())
	{
		const TArray<FName> Selected = GetSelectedPoses();
		bCanMirror = !Selected.IsEmpty();
		for (const FName Pose : Selected)
		{
			if (!FFVPoseMirror::CanMirrorPose(*Asset, Pose, MirrorReason))
			{
				bCanMirror = false;
				break;
			}
		}
	}
}

void SFVMeatbagFactory::SelectPoses(const TArray<FName>& Poses)
{
	if (!PoseList.IsValid())
	{
		return;
	}
	PoseList->ClearSelection();
	for (const FPoseItemPtr& Item : Items)
	{
		if (Poses.Contains(Item->Name))
		{
			PoseList->SetItemSelection(Item, true);
		}
	}
}

TArray<FName> SFVMeatbagFactory::GetSelectedPoses() const
{
	TArray<FName> Poses;
	if (PoseList.IsValid())
	{
		for (const FPoseItemPtr& Item : PoseList->GetSelectedItems())
		{
			Poses.Add(Item->Name);
		}
	}
	return Poses;
}

FName SFVMeatbagFactory::GetSinglePose() const
{
	const TArray<FName> Poses = GetSelectedPoses();
	return Poses.Num() == 1 ? Poses[0] : NAME_None;
}

UDebugSkelMeshComponent* SFVMeatbagFactory::GetPreviewComponent() const
{
	const TSharedPtr<IPersonaToolkit> Toolkit = PersonaToolkit.Pin();
	return Toolkit.IsValid() ? Toolkit->GetPreviewMeshComponent() : nullptr;
}

UAnimPreviewInstance* SFVMeatbagFactory::GetPreviewInstance() const
{
	const UDebugSkelMeshComponent* Component = GetPreviewComponent();
	return Component ? Component->PreviewInstance.Get() : nullptr;
}

void SFVMeatbagFactory::ApplyPreview(bool bAssetChanged)
{
	UDebugSkelMeshComponent* Component = GetPreviewComponent();
	UAnimPreviewInstance* Instance = GetPreviewInstance();
	UPoseAsset* Asset = GetPoseAsset();
	if (!Component || !Instance)
	{
		return;
	}

	bool bAnyWeight = false;
	for (const TPair<FName, float>& Weight : Weights)
	{
		bAnyWeight |= Weight.Value > 0.f;
	}

	if (Asset && bAnyWeight)
	{
		if (bAssetChanged || Instance->GetCurrentAsset() != Asset)
		{
			Component->EnablePreview(true, Asset);
		}
		for (const FName Pose : Asset->GetPoseFNames())
		{
			Instance->SetPreviewCurveOverride(Pose, Weights.FindRef(Pose), true);
		}
	}
	else if (Instance->GetCurrentAsset() && Instance->GetCurrentAsset() == Asset)
	{
		for (const FName Pose : Asset->GetPoseFNames())
		{
			Instance->SetPreviewCurveOverride(Pose, 0.f, true);
		}
		Component->EnablePreview(true, nullptr);
	}
}

void SFVMeatbagFactory::GoTo(FName Pose)
{
	if (UAnimPreviewInstance* Instance = GetPreviewInstance())
	{
		Instance->ResetModifiedBone();
	}
	Weights.Reset();
	Weights.Add(Pose, 1.f);
	ActivePose = Pose;
	ApplyPreview(true);
}

FFVPoseSnapshot SFVMeatbagFactory::CaptureViewport() const
{
	const UPoseAsset* Asset = GetPoseAsset();
	FFVPoseSnapshot Snapshot;
	if (!Asset || ActivePose.IsNone() || !FFVPoseAuthoring::ReadPose(*Asset, ActivePose, Snapshot))
	{
		Snapshot = Asset ? FFVPoseAuthoring::ReadBasePose(*Asset) : FFVPoseSnapshot();
	}

	// Persona's bone gizmo writes replace-mode, bone-space controllers on the preview instance.
	if (UAnimPreviewInstance* Instance = GetPreviewInstance())
	{
		for (const FAnimNode_ModifyBone& Controller : Instance->GetBoneControllers())
		{
			Snapshot.Bones.Add(Controller.BoneToModify.BoneName, FTransform(Controller.Rotation.Quaternion(), Controller.Translation, Controller.Scale));
		}
	}
	return Snapshot;
}

TArray<FName> SFVMeatbagFactory::GetSelectedBones() const
{
	TArray<FName> Bones;
	if (const UDebugSkelMeshComponent* Component = GetPreviewComponent())
	{
		if (const USkeletalMesh* Mesh = Component->GetSkeletalMeshAsset())
		{
			for (const int32 Bone : Component->BonesOfInterest)
			{
				if (Mesh->GetRefSkeleton().IsValidIndex(Bone))
				{
					Bones.Add(Mesh->GetRefSkeleton().GetBoneName(Bone));
				}
			}
		}
	}
	return Bones;
}

void SFVMeatbagFactory::Report(bool bSuccess, const FText& Error, const FText& Done)
{
	if (bSuccess && Done.IsEmpty())
	{
		return;
	}
	FNotificationInfo Info(bSuccess ? Done : Error);
	Info.ExpireDuration = bSuccess ? 3.f : 6.f;
	if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
	{
		Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}
}

void SFVMeatbagFactory::AfterEdit(const TArray<FName>& Select)
{
	Refresh();
	SelectPoses(Select);
	RefreshCurves();
	ApplyPreview(true);
}

void SFVMeatbagFactory::CreatePose()
{
	UPoseAsset* Asset = GetPoseAsset();
	const FName Pose(*NewPoseName.TrimStartAndEnd());
	FText Error;
	if (Asset && FFVPoseAuthoring::CreatePose(*Asset, Pose, CaptureViewport(), Error))
	{
		FSlateApplication::Get().DismissAllMenus();
		GoTo(Pose);
		AfterEdit({ Pose });
		return;
	}
	Report(false, Error);
}

FReply SFVMeatbagFactory::UpdatePose()
{
	const FName Pose = GetSinglePose();
	FText Error;
	const bool bDone = FFVPoseAuthoring::UpdatePose(*GetPoseAsset(), Pose, CaptureViewport(), Error);
	Report(bDone, Error);
	if (bDone)
	{
		GoTo(Pose);
		AfterEdit({ Pose });
	}
	return FReply::Handled();
}

FReply SFVMeatbagFactory::DeletePoses()
{
	const TArray<FName> Poses = GetSelectedPoses();
	FText Error;
	const bool bDone = FFVPoseAuthoring::DeletePoses(*GetPoseAsset(), Poses, Error);
	Report(bDone, Error);
	if (bDone)
	{
		for (const FName Pose : Poses)
		{
			Weights.Remove(Pose);
		}
		if (Poses.Contains(ActivePose))
		{
			ActivePose = NAME_None;
		}
		AfterEdit({});
	}
	return FReply::Handled();
}

FReply SFVMeatbagFactory::GoToSelected()
{
	GoTo(GetSinglePose());
	return FReply::Handled();
}

FReply SFVMeatbagFactory::ResetPreview()
{
	if (UAnimPreviewInstance* Instance = GetPreviewInstance())
	{
		Instance->ResetModifiedBone();
	}
	Weights.Reset();
	ActivePose = NAME_None;
	ApplyPreview(false);
	return FReply::Handled();
}

FReply SFVMeatbagFactory::DuplicatePose()
{
	UPoseAsset* Asset = GetPoseAsset();
	const FName Pose = GetSinglePose();
	const FName Copy = FFVPoseAuthoring::MakeUniquePoseName(*Asset, Pose.ToString() + TEXT("_copy"));
	FText Error;
	const bool bDone = FFVPoseAuthoring::DuplicatePose(*Asset, Pose, Copy, Error);
	Report(bDone, Error);
	if (bDone)
	{
		AfterEdit({ Copy });
	}
	return FReply::Handled();
}

FReply SFVMeatbagFactory::MirrorSelected()
{
	TArray<FName> Targets;
	FText Error;
	const bool bDone = FFVPoseMirror::MirrorPoses(*GetPoseAsset(), GetSelectedPoses(), MirrorAxis, Targets, Error);
	Report(bDone, Error);
	if (bDone)
	{
		AfterEdit(Targets);
	}
	return FReply::Handled();
}

void SFVMeatbagFactory::MirrorAll(EFVBodySide From)
{
	UPoseAsset* Asset = GetPoseAsset();
	if (!Asset)
	{
		return;
	}
	const FFVMirrorNameRule Rule = FFVMirrorNameRule::FromSettings();
	TArray<FName> Poses;
	int32 Skipped = 0;
	for (const FName Pose : Asset->GetPoseFNames())
	{
		FText Reason;
		if (Rule.GetNameSide(Pose.ToString()) == From)
		{
			if (FFVPoseMirror::CanMirrorPose(*Asset, Pose, Reason))
			{
				Poses.Add(Pose);
			}
			else
			{
				++Skipped;
			}
		}
	}

	TArray<FName> Targets;
	FText Error;
	const bool bDone = FFVPoseMirror::MirrorPoses(*Asset, Poses, MirrorAxis, Targets, Error);
	Report(bDone, Error, FText::Format(LOCTEXT("MirroredAll", "Mirrored {0} poses, skipped {1} that can't be mirrored."), Targets.Num(), Skipped));
	if (bDone)
	{
		AfterEdit(Targets);
	}
}

void SFVMeatbagFactory::CreateInbetweens()
{
	TArray<FName> Created;
	FText Error;
	const bool bDone = GetPoseAsset() && FFVPoseAuthoring::CreateInbetweens(*GetPoseAsset(), GetSinglePose(), InbetweenCount, Created, Error);
	Report(bDone, Error);
	if (bDone)
	{
		FSlateApplication::Get().DismissAllMenus();
		AfterEdit(Created);
	}
}

void SFVMeatbagFactory::Copy(bool bSelectedBones)
{
	FFVPoseSnapshot Snapshot;
	if (!GetPoseAsset() || !FFVPoseAuthoring::ReadPose(*GetPoseAsset(), GetSinglePose(), Snapshot))
	{
		return;
	}
	TSet<FName> Bones;
	if (bSelectedBones)
	{
		Bones.Append(GetSelectedBones());
		if (Bones.IsEmpty())
		{
			Report(false, LOCTEXT("ErrNoBones", "Select bones in the skeleton tree first."));
			return;
		}
	}
	Clipboard = MoveTemp(Snapshot);
	ClipboardBones = MoveTemp(Bones);
	Report(true, FText::GetEmpty(), FText::Format(LOCTEXT("Copied", "Copied {0}."), FText::FromName(GetSinglePose())));
}

void SFVMeatbagFactory::Paste()
{
	FText Error;
	const TArray<FName> Targets = GetSelectedPoses();
	const bool bDone = GetPoseAsset() && Clipboard.IsSet()
		&& FFVPoseAuthoring::PastePose(*GetPoseAsset(), Targets, Clipboard.GetValue(), ClipboardBones, PasteMultiplier, Error);
	Report(bDone, Error);
	if (bDone)
	{
		AfterEdit(Targets);
	}
}

void SFVMeatbagFactory::RenamePose(const FText& Text, ETextCommit::Type Commit, FName Pose)
{
	const FName NewName(*Text.ToString().TrimStartAndEnd());
	if (Commit == ETextCommit::OnCleared || NewName == Pose || !GetPoseAsset())
	{
		return;
	}
	FText Error;
	const bool bDone = FFVPoseAuthoring::RenamePose(*GetPoseAsset(), Pose, NewName, Error);
	Report(bDone, Error);
	if (bDone)
	{
		if (const float* Weight = Weights.Find(Pose))
		{
			Weights.Add(NewName, *Weight);
			Weights.Remove(Pose);
		}
		if (ActivePose == Pose)
		{
			ActivePose = NewName;
		}
		AfterEdit({ NewName });
	}
	else
	{
		Refresh();
	}
}

void SFVMeatbagFactory::SetCurveValue(float Value, ETextCommit::Type Commit, FName Curve)
{
	const FName Pose = GetSinglePose();
	if (!GetPoseAsset() || Pose.IsNone() || FMath::IsNearlyEqual(CurveValues.FindRef(Curve), Value))
	{
		return;
	}
	FText Error;
	const bool bDone = FFVPoseAuthoring::SetCurveValue(*GetPoseAsset(), Pose, Curve, Value, Error);
	Report(bDone, Error);
	AfterEdit({ Pose });
}

void SFVMeatbagFactory::AddCurve(FName Curve)
{
	FText Error;
	const bool bDone = GetPoseAsset() && FFVPoseAuthoring::AddCurve(*GetPoseAsset(), Curve, Error);
	Report(bDone, Error);
	if (bDone)
	{
		AfterEdit(GetSelectedPoses());
	}
}

bool SFVMeatbagFactory::HasAsset() const
{
	return PoseAsset.IsValid() && GetPreviewComponent() != nullptr;
}

bool SFVMeatbagFactory::HasSinglePose() const
{
	return HasAsset() && !GetSinglePose().IsNone();
}

bool SFVMeatbagFactory::HasSelection() const
{
	return HasAsset() && !GetSelectedPoses().IsEmpty();
}

bool SFVMeatbagFactory::CanMirror() const
{
	return HasSelection() && bCanMirror;
}

FText SFVMeatbagFactory::GetMirrorToolTip() const
{
	return MirrorReason.IsEmpty()
		? LOCTEXT("MirrorTip", "Mirror the selected poses' side bones onto their counterpart poses, updating or creating them. Select poses whose names have a side token, such as _l.")
		: MirrorReason;
}

#undef LOCTEXT_NAMESPACE
