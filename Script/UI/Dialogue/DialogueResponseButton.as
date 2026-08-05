delegate void FOnChoiceButtonClicked(int ResponseID);

class UDialogueResponseButton : UUserWidget
{
    UPROPERTY(BindWidget)
    UButton RootButton;

    UPROPERTY(BindWidget)
    UTextBlock ResponseText;

    UPROPERTY(BindWidget)
    UImage IconImage;

    UPROPERTY(BindWidget)
    UImage LockedOverlay;

    private int ChoiceIndex;

    UPROPERTY()
    FOnChoiceButtonClicked OnResponseClicked;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        RootButton.OnClicked.AddUFunction(this, n"HandleClicked");
    }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetupFromResponse(FFVDialogueChoiceMessage Choice)
    {
        ChoiceIndex = Choice.Index;
        ResponseText.SetText(Choice.Text);

        // No icons for now. Future logic might be getting a response type and select
        // an icon from a data table.
        IconImage.SetVisibility(ESlateVisibility::Collapsed);

        if (Choice.bWasTakenBefore)
        {
            RootButton.SetIsEnabled(false);
            LockedOverlay.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            SetRenderOpacity(0.45f);
        }
        else
        {
            RootButton.SetIsEnabled(true);
            LockedOverlay.SetVisibility(ESlateVisibility::Collapsed);
            SetRenderOpacity(1.f);
        }
    }

    UFUNCTION()
    private void HandleClicked()
    {
        if (ChoiceIndex != -1)
        {
            OnResponseClicked.Execute(ChoiceIndex);
        }
    }
}