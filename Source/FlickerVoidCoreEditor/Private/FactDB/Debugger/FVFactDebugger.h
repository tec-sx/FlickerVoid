#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FactDB/FVFactTypes.h"
#include "FactDB/Debugger/FVFactDebuggerSettingsLocal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

struct FFVSearchToggleState;
class UFVFactPreset;
class SFVFactSearchToggle;
class SWrapBox;
class SFVFactSearchBox;
class SComboButton;

using FFVFactTreeItemRef = TSharedRef<struct FFVFactTreeItem>;
using FFVFactTreeItemPtr = TSharedPtr<struct FFVFactTreeItem>;

struct FFVFactTreeItem : public TSharedFromThis<FFVFactTreeItem>
{
	FGameplayTag Tag;
	FName SimpleTagName;
	TArray<FFVFactTreeItemPtr> Children;

	TOptional<int32> Value;
	float ValueChangedTime = 0;

	~FFVFactTreeItem();

	void StartPlay();
	void EndPlay();
	void InitItem(bool bPlayAnimation = false);

	void HandleValueChanged(int32 NewValue);
	void HandleUndefined();
	void HandleNewValueCommited(int32 NewValue, ETextCommit::Type Type) const;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFactItemValueChanged, FGameplayTag, int32)
	FOnFactItemValueChanged OnFactItemValueChanged;
};


class SFVFactDebugger : public SCompoundWidget
{
	using SFactsTreeView = STreeView<TSharedPtr<FFVFactTreeItem>>;

public:
	SLATE_BEGIN_ARGS(SFVFactDebugger)
		{
		}

		SLATE_ARGUMENT(bool, bIsGameStarted)
	SLATE_END_ARGS()


	virtual void Construct(const FArguments& InArgs);
	virtual ~SFVFactDebugger() override;

private:
	// Play started
	void HandleGameInstanceStarted();
	void HandleGameInstanceEnded();
	void InitItem(const FFVFactTreeItemPtr& Item);
	void ResetItem(const FFVFactTreeItemPtr& Item);

	TSharedRef<SWidget> CreateLeftToolBar();
	TSharedRef<SWidget> CreateRightToolBar();

	// Create tree widgets
	TSharedRef<SWidget> CreateTreeLabel(const FText& InLabel) const;
	TSharedRef<SWidget> CreateFactsTree(bool bIsFavoritesTree);
	TSharedRef<SHeaderRow> CreateHeaderRow(bool bIsFavoritesTree) const;
	TSharedRef<SWidget> CreateFilterStatusWidget(bool bIsFavoritesTree) const;

	TSharedRef<ITableRow> OnGenerateWidgetForFactsTreeView(FFVFactTreeItemPtr FactTreeItem,
	                                                       const TSharedRef<STableViewBase>& TableViewBase);
	TSharedRef<ITableRow> HandleGeneratePinnedTreeRow(FFVFactTreeItemPtr FactTreeItem,
	                                                  const TSharedRef<STableViewBase>& TableViewBase);
	void OnGetChildren(FFVFactTreeItemPtr FactTreeItem, TArray<FFVFactTreeItemPtr>& Children);
	void HandleExpansionChanged(FFVFactTreeItemPtr FactTreeItem, bool bInExpanded, bool bRecursive,
	                            bool bIsFavoritesTree);
	void HandleMainExpansionChanged(FFVFactTreeItemPtr FactTreeItem, bool bInExpanded, bool bRecursive);
	void HandleFavoritesExpansionChanged(FFVFactTreeItemPtr FactTreeItem, bool bInExpanded, bool bRecursive);

	FText GetFilterStatusText(bool bIsFavoritesTree) const;
	FSlateColor GetFilterStatusTextColor(bool bIsFavoritesTree) const;

	TSharedRef<SWidget> HandleGeneratePresetsMenu();
	void HandlePresetSelected(const UFVFactPreset* Preset);
	TSharedRef<SWidget> HandleGenerateOptionsMenu();
	void GenerateCommonContextMenu(FMenuBuilder& MenuBuilder, bool bIsFavoritesTree);
	TSharedPtr<SWidget> HandleGenerateMainContextMenu();
	TSharedPtr<SWidget> HandleGenerateFavoritesContextMenu();

	void ClearFavoritesRecursive(const FFVFactTreeItemPtr& Item) const;
	bool HasFavoritesRecursive(const FFVFactTreeItemPtr& Item) const;
	void PostFavoritesChanged();

	// Searching and filtering
	void HandleSearchTextChanged(const FText& SearchText);
	void HandleSaveSearchClicked(const FText& SearchText);
	void FilterItems();

	static int32 CountAllMainItems(const FFVFactTreeItemPtr& ParentNode);
	static int32 CountAllFavoriteItems(const FFVFactTreeItemPtr& ParentNode, bool bIsParentFavorite);

	// Options menu
	void HandleExpandAllClicked(bool bExpandMain, bool bExpandFavorites);
	void HandleCollapseAllClicked(bool bCollapseMain, bool bCollapseFavorites);

	// Items expansion
	void SetItemsExpansion(const TSharedPtr<SFactsTreeView>& TreeView,
	                       const TArray<FFVFactTreeItemPtr>& FactItems,
	                       bool bShouldExpand,
	                       bool bPersistExpansion);
	void SetDefaultMainItemsExpansion(const TArray<FFVFactTreeItemPtr>& FactItems);
	void SetDefaultFavoriteItemsExpansion(const TArray<FFVFactTreeItemPtr>& FactItems);

	static bool FindItemByTagRecursive(const FFVFactTreeItemPtr& Item,
	                                   const FGameplayTag Tag,
	                                   TArray<FFVFactTreeItemPtr>& OutPath);

	// Search toggles
	void CreateDefaultSearchToggles(TArray<FFVSearchToggleState> SearchToggleStates);
	TSharedRef<SFVFactSearchToggle> ConstructSearchToggle(const FText& InSearchText, bool bInChecked = false);

	FReply HandleRemoveSearchToggle();
	void CleanupSearchesMarkedForDelete();
	void RefreshSearchToggles();
	FReply HandleClearTogglesClicked();
	FReply HandleSearchToggleClicked();

	TArray<FFVSearchToggleState> GetSearchToggleStates() const;
	bool IsAnySearchToggleActive() const;

	// Build items
	void BuildFactTreeItems(bool bPlayAnimation = false);
	FFVFactTreeItemPtr BuildFactItem(const FFVFactTreeItemPtr& ParentNode, const TSharedPtr<FGameplayTagNode>& ThisNode,
	                               bool bPlayAnimation);
	void RebuildFactTreeItems(bool bPlayAnimation = false);
	void HandleFactValueChanged(FGameplayTag FactTag, int32 NewValue);
	void HandleAnyFactChanged(FGameplayTag FactTag, int32 NewValue, EFVFactChangeReason Reason);
	static void DispatchToItemRecursive(const FFVFactTreeItemPtr& Item, const FGameplayTag FactTag, int32 NewValue,
										bool bUndefined);

	// Settings
	void LoadSettings();
	void SaveSettings() const;

	void HandleOrientationChanged(EOrientation Orientation) const;

public:
	static TArray<FGameplayTag> FavoriteFacts;

private:
	TSharedPtr<SSplitter> Splitter;
	TSharedPtr<SFactsTreeView> MainTreeView;
	TSharedPtr<SFactsTreeView> FavoriteTreeView;

	FFVFactTreeItemPtr RootItem;
	FFVFactTreeItemPtr MainTreeItem;
	FFVFactTreeItemPtr FavoritesTreeItem;

	int32 AllMainFactsCount = 0;
	int32 AllFavoriteFactsCount = 0;

	int32 CurrentMainFactsCount = 0;
	int32 CurrentFavoriteFactsCount = 0;

	TSharedPtr<SFVFactSearchBox> SearchBox;
	TSharedPtr<SComboButton> OptionsButton;

	TSharedPtr<SHorizontalBox> SearchesHBox;
	TSharedPtr<SWrapBox> SearchesContainer;
	TArray<TSharedRef<SFVFactSearchToggle>> CurrentSearchToggles;
	FText CurrentSearchText;

	// Save expansion state for tag item. The expansion state does not persist between editor sessions. 
	static TSet<FGameplayTag> MainExpandedFacts;
	static TSet<FGameplayTag> FavoriteCollapsedFacts;
	bool bPersistExpansionChange = true;

	bool bDisplayOnlyPinnedItems = false;

#if WITH_EDITOR
	FDelegateHandle TagChangedHandle;
#endif
	FDelegateHandle FactsLoadedHandle;
	FDelegateHandle AnyFactChangedHandle;

	bool bIsPlaying = false;
};
