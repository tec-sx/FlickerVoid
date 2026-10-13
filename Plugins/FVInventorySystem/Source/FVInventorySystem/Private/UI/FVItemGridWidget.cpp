#include "UI/FVItemGridWidget.h"

#include "Components/PanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "FVInventorySystem.h"
#include "UI/FVItemEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemGridWidget)

void UFVItemGridWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	EnsureCells(FMath::Max(CellCount, Entries.Num()));
}

void UFVItemGridWidget::SetItems(const TArray<FFVItemStack>& Items)
{
	const bool bHadHover = HoveredEntry.IsValid();
	EnsureCells(FMath::Max(CellCount, Items.Num()));

	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Items.IsValidIndex(i))
		{
			Entries[i]->SetItem(Items[i]);
		}
		else
		{
			Entries[i]->ClearItem();
		}
	}

	SetSelectedEntry(FindEntry(SelectedId));

	UFVItemEntryWidget* Hovered = HoveredEntry.Get();
	if (Hovered || bHadHover)
	{
		OnHoveredItemChanged.Broadcast(Hovered, Hovered ? Hovered->GetItem() : FFVItemStack());
	}
}

void UFVItemGridWidget::ClearItems()
{
	SetItems(TArray<FFVItemStack>());
}

TArray<UFVItemEntryWidget*> UFVItemGridWidget::GetEntries() const
{
	return ObjectPtrDecay(Entries);
}

bool UFVItemGridWidget::GetHoveredItem(FFVItemStack& OutItem) const
{
	const UFVItemEntryWidget* Entry = HoveredEntry.Get();
	if (!Entry || Entry->IsEmpty())
	{
		return false;
	}
	OutItem = Entry->GetItem();
	return true;
}

UFVItemEntryWidget* UFVItemGridWidget::GetSelectedEntry() const
{
	for (UFVItemEntryWidget* Entry : Entries)
	{
		if (Entry->IsSelected())
		{
			return Entry;
		}
	}
	return nullptr;
}

bool UFVItemGridWidget::GetSelectedItem(FFVItemStack& OutItem) const
{
	const UFVItemEntryWidget* Entry = GetSelectedEntry();
	if (!Entry)
	{
		return false;
	}
	OutItem = Entry->GetItem();
	return true;
}

void UFVItemGridWidget::SelectItem(const FGuid InstanceId)
{
	SetSelectedEntry(FindEntry(InstanceId));
}

void UFVItemGridWidget::ClearSelection()
{
	SetSelectedEntry(nullptr);
}

UWidget* UFVItemGridWidget::GetFocusTarget() const
{
	if (UFVItemEntryWidget* Selected = GetSelectedEntry())
	{
		return Selected;
	}
	return Entries.Num() > 0 ? Entries[0].Get() : nullptr;
}

void UFVItemGridWidget::EnsureCells(const int32 Count)
{
	if (!ItemPanel)
	{
		return;
	}
	if (!EntryWidgetClass)
	{
		if (!IsDesignTime())
		{
			UE_LOG(LogFVInventorySystem, Warning, TEXT("%s: Entry Widget Class is not set."), *GetName());
		}
		return;
	}

	// Drops placeholder children left in the panel by the designer.
	if (Entries.Num() == 0)
	{
		ItemPanel->ClearChildren();
	}

	UUniformGridPanel* UniformGrid = Cast<UUniformGridPanel>(ItemPanel);
	const int32 RowLength = FMath::Max(1, Columns);

	while (Entries.Num() < Count)
	{
		UFVItemEntryWidget* Entry = CreateWidget<UFVItemEntryWidget>(this, EntryWidgetClass);
		if (!Entry)
		{
			return;
		}

		const int32 Index = Entries.Num();
		Entry->SetIndex(Index);
		Entry->OnHovered.AddDynamic(this, &ThisClass::HandleEntryHovered);
		Entry->OnUnhovered.AddDynamic(this, &ThisClass::HandleEntryUnhovered);
		Entry->OnClicked.AddDynamic(this, &ThisClass::HandleEntryClicked);

		if (UniformGrid)
		{
			UniformGrid->AddChildToUniformGrid(Entry, Index / RowLength, Index % RowLength);
		}
		else
		{
			ItemPanel->AddChild(Entry);
		}

		Entries.Add(Entry);
		Entry->ClearItem();
	}

	while (Entries.Num() > Count)
	{
		RemoveLastCell();
	}
}

void UFVItemGridWidget::RemoveLastCell()
{
	UFVItemEntryWidget* Entry = Entries.Pop();
	const bool bHadFocus = Entry->HasAnyUserFocus();

	Entry->OnHovered.RemoveAll(this);
	Entry->OnUnhovered.RemoveAll(this);
	Entry->OnClicked.RemoveAll(this);
	Entry->RemoveFromParent();

	if (HoveredEntry == Entry)
	{
		HoveredEntry = nullptr;
	}
	if (bHadFocus && Entries.Num() > 0)
	{
		Entries.Last()->SetFocus();
	}
}

void UFVItemGridWidget::SetSelectedEntry(UFVItemEntryWidget* Entry)
{
	if (Entry && Entry->IsEmpty())
	{
		Entry = nullptr;
	}

	const UFVItemEntryWidget* Previous = GetSelectedEntry();
	SelectedId = Entry ? Entry->GetItem().InstanceId : FGuid();
	for (UFVItemEntryWidget* Cell : Entries)
	{
		Cell->SetSelected(Cell == Entry);
	}

	if (Previous != Entry)
	{
		OnSelectionChanged.Broadcast(Entry, Entry ? Entry->GetItem() : FFVItemStack());
	}
}

UFVItemEntryWidget* UFVItemGridWidget::FindEntry(const FGuid& InstanceId) const
{
	if (!InstanceId.IsValid())
	{
		return nullptr;
	}
	for (UFVItemEntryWidget* Entry : Entries)
	{
		if (!Entry->IsEmpty() && Entry->GetItem().InstanceId == InstanceId)
		{
			return Entry;
		}
	}
	return nullptr;
}

void UFVItemGridWidget::HandleEntryHovered(UFVItemEntryWidget* Entry)
{
	if (HoveredEntry.Get() != Entry)
	{
		HoveredEntry = Entry;
		OnHoveredItemChanged.Broadcast(Entry, Entry->GetItem());
	}
}

void UFVItemGridWidget::HandleEntryUnhovered(UFVItemEntryWidget* Entry)
{
	if (HoveredEntry.Get() != Entry)
	{
		return;
	}

	// The mouse can leave one cell while another still holds keyboard/gamepad focus.
	UFVItemEntryWidget* Next = nullptr;
	for (UFVItemEntryWidget* Cell : Entries)
	{
		if (Cell != Entry && Cell->IsItemHovered())
		{
			Next = Cell;
			break;
		}
	}

	HoveredEntry = Next;
	OnHoveredItemChanged.Broadcast(Next, Next ? Next->GetItem() : FFVItemStack());
}

void UFVItemGridWidget::HandleEntryClicked(UFVItemEntryWidget* Entry)
{
	if (!Entry->IsEmpty())
	{
		SetSelectedEntry(Entry);
	}
	OnItemClicked.Broadcast(Entry, Entry->GetItem());
}
