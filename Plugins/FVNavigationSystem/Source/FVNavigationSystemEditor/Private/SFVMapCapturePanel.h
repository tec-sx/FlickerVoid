#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class AActor;
class AFVMapCaptureActor;
class IDetailsView;
class ITableRow;
class STableViewBase;

/** Lists the map capture actors in the open level, shows the selected one's settings and captures them. */
class SFVMapCapturePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFVMapCapturePanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFVMapCapturePanel() override;

private:
	struct FItem
	{
		TWeakObjectPtr<AFVMapCaptureActor> Actor;
	};

	void Refresh();
	void Capture(AFVMapCaptureActor* Actor);

	TSharedRef<ITableRow> MakeRow(TSharedPtr<FItem> Item, const TSharedRef<STableViewBase>& Owner);
	void HandleSelectionChanged(TSharedPtr<FItem> Item, ESelectInfo::Type SelectInfo);
	void HandleActorAdded(AActor* Actor);
	void HandleActorDeleted(AActor* Actor);
	void HandleMapChanged(uint32 MapChangeFlags);

	FReply HandleRefreshClicked();
	FReply HandleCaptureAllClicked();

	TArray<TSharedPtr<FItem>> Items;
	TSharedPtr<SListView<TSharedPtr<FItem>>> ListView;
	TSharedPtr<IDetailsView> DetailsView;
	FText Status;
};
