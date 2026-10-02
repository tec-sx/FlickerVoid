UCLASS(Abstract, Blueprintable)
class UInteractionOfferWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UImage IconImage;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ActionNameText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock KeyHintText;

    void SetSlotData(FFVInteractionOfferData Offer, const FInteractionSlotStyle& Style)
    {
        SetVisibility(ESlateVisibility::HitTestInvisible);

        if (ActionNameText != nullptr)
        {
            Offer.Display.Name
        }

        SetRenderOpacity(Offer.bAvailable ? 1.f : 0.4f);
        SetToolTipText(Offer.LockedReason);
    }

    void Clear()
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}
