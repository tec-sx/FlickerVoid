// AngelScript implementation of the interaction prompt overlay.
//
// Setup in the UMG designer:
//   1. Create a Widget Blueprint that inherits this class (or UFVInteractionPromptWidget).
//   2. Add 4 child UFVInteractionSlotWidget children named Slot0..Slot3.
//   3. Fill InputTagHints with the display text for each of the 4 input tags
//      (e.g. InputTag.Interact.Primary → "E", InputTag.Interact.PrimaryHold → "Hold E").
//   4. Assign this widget class to AFVHUD::InteractionPromptWidgetClass in the HUD blueprint.
//
// Visibility: the widget is always on the viewport but Hidden when no interactable is focused.
// Individual slots beyond the action count are also Hidden.

class UFVActionsPromptWidget : UFVInteractionPromptWidget
{
    // Designer-configured: maps input slot tag → short key hint string shown in the slot
    // Example entries:
    //   InputTag.Interact.Primary       → "E"
    //   InputTag.Interact.PrimaryHold   → "[Hold] E"
    //   InputTag.Interact.Secondary     → "F"
    //   InputTag.Interact.SecondaryHold → "[Hold] F"
    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Input Hints")
    TMap<FGameplayTag, FText> InputTagHints;

    // Four action slots — bind these in the UMG designer by naming the child
    // widgets Slot0, Slot1, Slot2, Slot3 (must be UFVInteractionSlotWidget instances).
    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot0;

    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot1;

    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot2;

    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot3;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        SetVisibility(ESlateVisibility::Hidden);
    }

    // Called by the C++ base whenever focus changes.
    UFUNCTION(BlueprintOverride)
    void OnFocusChanged(
		UFVInteractableComponent FocusedInteractable,
		const TArray<FFVInteractionActionDisplay>& Actions)
    {
        if (FocusedInteractable == nullptr || Actions.Num() == 0)
        {
            SetVisibility(ESlateVisibility::Hidden);
            return;
        }

        SetVisibility(ESlateVisibility::HitTestInvisible);
        UpdateSlot(Slot0, Actions, 0);
        UpdateSlot(Slot1, Actions, 1);
        UpdateSlot(Slot2, Actions, 2);
        UpdateSlot(Slot3, Actions, 3);
    }

    private void UpdateSlot(
		UFVInteractionSlotWidget Slot,
		const TArray<FFVInteractionActionDisplay>& Actions,
		int32 Index)
    {
        if (Slot == nullptr)
        {
            return;
        }

        if (Index < Actions.Num())
        {
            FText Hint = GetHintForTag(Actions[Index].InputTag);
            Slot.SetSlotData(Actions[Index], Hint);
            Slot.SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            Slot.SetVisibility(ESlateVisibility::Hidden);
        }
    }

    private FText GetHintForTag(FGameplayTag Tag)
    {
        FText HintText;
        if (InputTagHints.Find(Tag, HintText))
        {
            return HintText;
        }
        return FText::FromString("?");
    }
}
