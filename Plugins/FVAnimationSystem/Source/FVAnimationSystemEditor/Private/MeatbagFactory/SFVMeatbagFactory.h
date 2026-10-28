#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "FVAnimationTypes.h"
#include "PoseAuthoring/FVPoseAuthoring.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class IPersonaToolkit;
class ITableRow;
class STableViewBase;
class UAnimPreviewInstance;
class UDebugSkelMeshComponent;
class UPoseAsset;
struct FAssetData;

/**
 * The Meatbag Factory window: authors and mirrors the poses of one Pose Asset on the Persona preview mesh.
 * Create and Update save the pose last opened with Go To (or the base pose) plus the bones edited in the viewport;
 * post-process correctives on the preview are never captured. Preview weights only change the viewport.
 */
class SFVMeatbagFactory : public SCompoundWidget, public FSelfRegisteringEditorUndoClient
{
public:
	SLATE_BEGIN_ARGS(SFVMeatbagFactory) {}
		SLATE_ARGUMENT(TSharedPtr<IPersonaToolkit>, PersonaToolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFVMeatbagFactory() override;

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	struct FPoseItem
	{
		FName Name;
		EFVBodySide Side = EFVBodySide::Centre;
	};
	using FPoseItemPtr = TSharedPtr<FPoseItem>;
	using FCurveItemPtr = TSharedPtr<FName>;

	TSharedRef<SWidget> BuildSetup();
	TSharedRef<SWidget> BuildPoses();
	TSharedRef<SWidget> BuildActions();
	TSharedRef<ITableRow> MakePoseRow(FPoseItemPtr Item, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<ITableRow> MakeCurveRow(FCurveItemPtr Curve, const TSharedRef<STableViewBase>& Owner);
	TSharedRef<SWidget> MakeAxisMenu();
	TSharedRef<SWidget> MakeCreateMenu();
	TSharedRef<SWidget> MakeSideMenu();
	TSharedRef<SWidget> MakeMirrorMenu();
	TSharedRef<SWidget> MakeInbetweensMenu();
	TSharedRef<SWidget> MakeClipboardMenu();
	TSharedRef<SWidget> MakeAddCurveMenu();

	UPoseAsset* GetPoseAsset() const;
	FString GetPoseAssetPath() const;
	void OnPoseAssetChanged(const FAssetData& AssetData);
	bool ShouldFilterAsset(const FAssetData& AssetData) const;
	FText GetAxisText() const;
	FText GetSideText() const;
	FText GetMirrorAllText() const;

	void Refresh();
	void RefreshCurves();
	void SelectPoses(const TArray<FName>& Poses);
	TArray<FName> GetSelectedPoses() const;
	FName GetSinglePose() const;

	UDebugSkelMeshComponent* GetPreviewComponent() const;
	UAnimPreviewInstance* GetPreviewInstance() const;
	void ApplyPreview(bool bAssetChanged);
	void GoTo(FName Pose);
	FFVPoseSnapshot CaptureViewport() const;
	TArray<FName> GetSelectedBones() const;
	void Report(bool bSuccess, const FText& Error, const FText& Done = FText::GetEmpty());
	void AfterEdit(const TArray<FName>& Select);

	void CreatePose();
	FReply UpdatePose();
	FReply DeletePoses();
	FReply GoToSelected();
	FReply ResetPreview();
	FReply DuplicatePose();
	FReply MirrorSelected();
	FReply MirrorAll();
	void CreateInbetweens();
	void Copy(bool bSelectedBones);
	void Paste();
	void RenamePose(const FText& Text, ETextCommit::Type Commit, FName Pose);
	void SetCurveValue(float Value, ETextCommit::Type Commit, FName Curve);
	void AddCurve(FName Curve);

	bool HasAsset() const;
	bool HasSinglePose() const;
	bool HasSelection() const;
	bool CanMirror() const;
	FText GetMirrorToolTip() const;

	TWeakPtr<IPersonaToolkit> PersonaToolkit;
	TWeakObjectPtr<UPoseAsset> PoseAsset;
	EFVMirrorAxis MirrorAxis = EFVMirrorAxis::Default;
	EFVBodySide AuthoringSide = EFVBodySide::Left;

	FString Filter;
	TArray<FPoseItemPtr> Items;
	TSharedPtr<SListView<FPoseItemPtr>> PoseList;
	TArray<FCurveItemPtr> CurveItems;
	TMap<FName, float> CurveValues;
	TSharedPtr<SListView<FCurveItemPtr>> CurveList;

	TMap<FName, float> Weights;
	FName ActivePose;
	FString NewPoseName;
	int32 InbetweenCount = 1;
	float PasteMultiplier = 1.f;
	TOptional<FFVPoseSnapshot> Clipboard;
	TSet<FName> ClipboardBones;

	bool bCanMirror = false;
	FText MirrorReason;
};
