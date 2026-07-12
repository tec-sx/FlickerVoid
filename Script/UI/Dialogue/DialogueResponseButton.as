delegate void FOnResponseButtonClicked(FName ResponseID);

// ═════════════════════════════════════════════════════════════════════════════
// 1. UDialogueResponseButton
//
// Sub-widget representing one player response option in the choice list.
// Created dynamically by UDialogueWidget for each FEvaluatedResponse entry.
// The Blueprint child (WBP_ResponseButton) binds named widget properties.
// ═════════════════════════════════════════════════════════════════════════════
class UDialogueResponseButton : UUserWidget
{
    // The root button — we bind OnClicked to here in NativeConstruct
    UPROPERTY(BindWidget)
    UButton RootButton;

    UPROPERTY(BindWidget)
    UTextBlock ResponseText;

    UPROPERTY(BindWidget)
    UImage IconImage;

    UPROPERTY(BindWidget)
    UImage LockedOverlay;

    // ── Data ──────────────────────────────────────────────────────────────────

    private FName ResponseID;

    UPROPERTY()
    FOnResponseButtonClicked OnResponseClicked;

    // ── Setup ─────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        RootButton.OnClicked.AddUFunction(this, n"HandleClicked");
    }

    /**
     * Called by UDialogueWidget after creating this sub-widget instance.
     * Populates all visual state from the evaluated response data.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetupFromResponse(const FFVEvaluatedResponse& Response)
    {
        ResponseID = Response.ResponseID;
        ResponseText.SetText(Response.DisplayText);

        if (!Response.TooltipText.IsEmpty())
        {
            SetToolTipText(Response.TooltipText);
        }

        if (Response.IconTag.IsValid())
        {
            IconImage.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            BP_SetIcon(Response.IconTag);
        }
        else
        {
            IconImage.SetVisibility(ESlateVisibility::Collapsed);
        }

        if (Response.bIsAvailable)
        {
            RootButton.SetIsEnabled(true);
            LockedOverlay.SetVisibility(ESlateVisibility::Collapsed);
            
            SetRenderOpacity(1.f);
        }
        else
        {
            RootButton.SetIsEnabled(false);
            LockedOverlay.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
            SetRenderOpacity(0.45f);
        }
    }

    // ── Click Handler ─────────────────────────────────────────────────────────

    UFUNCTION()
    private void HandleClicked()
    {

        if (ResponseID == NAME_None) return;
        OnResponseClicked.Execute(ResponseID);
    }

    // ── Blueprint Events (implement visuals in WBP_ResponseButton) ────────────

    /**
     * Override in WBP_ResponseButton to set IconImage.Brush from a texture
     * looked up by the IconTag. Keeps texture asset references out of code.
     *
     * Example tags: UI.Icon.Question, UI.Icon.Trade, UI.Icon.Threat, UI.Icon.Flirt
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue")
    void BP_SetIcon(FGameplayTag IconTag) {}
}