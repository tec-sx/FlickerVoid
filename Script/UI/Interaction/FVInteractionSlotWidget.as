// Single action slot in the interaction prompt overlay.
// Bind child widgets by matching their name in the UMG designer:
//   ActionNameText   → UTextBlock showing the action label
//   KeyHintText      → UTextBlock showing the mapped key (e.g. "E", "Hold E")
//   IconImage        → UImage for the action icon (hidden when no icon set)

class UFVInteractionSlotWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UTextBlock ActionNameText;

    UPROPERTY(BindWidget)
    UTextBlock KeyHintText;

    UPROPERTY(BindWidgetOptional)
    UImage IconImage;

    // TODO: Update to use angelscript struct for easier iterations
    void SetSlotData(FFVInteractionActionInfo Data, FText KeyHint)
    {
        ActionNameText.SetText(Data.DisplayName);
        KeyHintText.SetText(KeyHint);

        if (IconImage != nullptr)
        {
            if (!Data.Icon.IsNull())
            {
                UTexture2D Tex = Data.Icon.Get();
                if (Tex != nullptr)
                {
                    IconImage.SetColorAndOpacity(FLinearColor::White);
                    IconImage.SetBrushFromTexture(Tex);
                    IconImage.SetVisibility(ESlateVisibility::HitTestInvisible);
                }
            }
            else
            {
                IconImage.SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        float Opacity = Data.bAvailable ? 1.f : 0.4f;
        SetRenderOpacity(Opacity);

        if (!Data.bAvailable && !Data.UnavailableReason.IsEmpty())
        {
            SetToolTipText(Data.UnavailableReason);
        }
    }
}
