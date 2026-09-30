UCLASS(Abstract, Blueprintable)
class UInteractionOfferWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UImage IconImage;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ActionNameText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock KeyHintText;

    void SetSlotData(FFVInteractionOffer Offer, const FInteractionSlotStyle& Style)
    {
        SetVisibility(ESlateVisibility::HitTestInvisible);

        // if (ActionNameText != nullptr)
        // {
        //     ActionNameText.SetText(Style.DisplayName);
        // }

        // if (IconImage != nullptr)
        // {
        //     IconImage.SetBrush(Style.Icon);
        //     IconImage.SetVisibility(ESlateVisibility::HitTestInvisible);
        // }

        // SetRenderOpacity(Offer.bRequirementsMet ? 1.f : 0.4f);
        // SetToolTipText(Offer.bRequirementsMet ? FText() : Style.RequirementHint);
    }

    void Clear()
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}
