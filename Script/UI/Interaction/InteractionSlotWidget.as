UCLASS(Abstract, Blueprintable)
class UInteractionSlotWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UImage IconImage;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ActionNameText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock KeyHintText;

    void SetSlotData(const FInteractionSlotStyle& Style, bool bEnabled)
    {
        SetVisibility(ESlateVisibility::HitTestInvisible);

        if (ActionNameText != nullptr)
        {
            ActionNameText.SetText(Style.DisplayName);
        }

        if (IconImage != nullptr)
        {
            IconImage.SetBrush(Style.Icon);
            IconImage.SetVisibility(ESlateVisibility::HitTestInvisible);
        }

        SetRenderOpacity(bEnabled ? 1.f : 0.4f);
        SetToolTipText(bEnabled ? FText() : Style.RequirementHint);
    }

    void Clear()
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}
