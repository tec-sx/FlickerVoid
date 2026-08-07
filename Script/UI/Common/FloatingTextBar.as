enum EColorScheme
{
    Default
}

class UFloatingTextBar : UUserWidget
{
    UPROPERTY(BindWidget)
    UTextBlock TextBlock;

    UPROPERTY(EditAnywhere)
    EColorScheme ColorScheme;
    
    private FTimerHandle HideTimerHandle;

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        System::ClearAndInvalidateTimerHandle(HideTimerHandle);
    }

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        Hide();
    }

    UFUNCTION()
    void SetText(FText Text)
    {
        if (IsValid(TextBlock) && !Text.IsEmptyOrWhitespace())
        {
            TextBlock.Text = Text;
        }
    }

    UFUNCTION()
    void Show()
    {
        if (IsValid(TextBlock) && !TextBlock.Text.IsEmptyOrWhitespace())
        {
            SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }

    UFUNCTION()
    void ShowForDuration(float Duration)
    {
        Show();
        HideTimerHandle = System::SetTimer(this, n"Hide", Duration, false);
    }

    UFUNCTION()
    void Hide()
    {
        SetVisibility(ESlateVisibility::Hidden);
    }
}