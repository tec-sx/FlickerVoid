// Single action slot in the interaction prompt overlay.
// Bind child widgets by matching their name in the UMG designer:
//   ActionNameText   → UTextBlock showing the action label
//   KeyHintText      → UTextBlock showing the mapped key (e.g. "E", "Hold E")
//   IconImage        → UImage for the action icon (hidden when no icon set)

class UFVInteractionSlotWidget : UUserWidget
{
    UPROPERTY(BindWidgetOptional)
    UImage IconImage;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ActionNameText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock KeyHintText;

    // Consumes pure presentation data produced by the interaction UI router.
    void SetSlotData(const FFVUIInteractionSlot& Data)
    {
        if (!Data.bOccupied)
        {
            SetVisibility(ESlateVisibility::Collapsed);
            return;
        }

        SetVisibility(ESlateVisibility::HitTestInvisible);

        if (ActionNameText != nullptr)
        {
            ActionNameText.SetText(Data.DisplayName);
        }

        if (KeyHintText != nullptr)
        {
            KeyHintText.SetText(Data.InputHint);
        }

        if (IconImage != nullptr)
        {
            UTexture2D Tex = Data.Icon.IsNull() ? nullptr : Data.Icon.Get();

            if (Tex != nullptr)
            {
                IconImage.SetColorAndOpacity(FLinearColor::White);
                IconImage.SetBrushFromTexture(Tex);
                IconImage.SetVisibility(ESlateVisibility::HitTestInvisible);
            }
            else
            {
                IconImage.SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        SetRenderOpacity(Data.bEnabled ? 1.f : 0.4f);
        SetToolTipText(Data.bEnabled ? FText() : Data.DisabledReason);
    }

    void Clear()
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}
