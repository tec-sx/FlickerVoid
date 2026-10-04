UCLASS(Abstract, Blueprintable)
class UInteractionSetWidget : UFVInteractionWidget
{
    UPROPERTY(BindWidget)
    UPanelWidget InteractionSetBox;

    // Drawn on the focused interactable. Place it centered in the widget; Tick offsets it from there.
    UPROPERTY(BindWidgetOptional)
    UImage FocusIcon;

    // Screen center reticle, only shown by interactor modes that ask for it (e.g. examine).
    UPROPERTY(BindWidgetOptional)
    UWidget CenterReticle;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotFirstWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotMidWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotLastWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration", Meta = (RequiredAssetDataTags = "RowStructure=/Script/Angelscript.InteractionSlotStyle"))
    UDataTable InteractionSlotStyles;


    private UFVInteractorComponent Interactor;
    private UFVInteractionUIComponent InteractionUI;
    private TArray<UInteractionSlotWidget> OfferPool;

    UFUNCTION(BlueprintOverride)
    void OnInteractionInitialized(UFVInteractorComponent InInteractor)
    {
        Interactor = InInteractor;
        InteractionUI = UFVInteractionUIComponent::Get(InInteractor.Owner);
        OfferPool.Empty();

        if (FocusIcon != nullptr)
        {
            FocusIcon.SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    UFUNCTION(BlueprintOverride)
    void Tick(FGeometry MyGeometry, float InDeltaTime)
    {
        if (FocusIcon == nullptr)
        {
            return;
        }

        FVector2D FocusPosition;
        if (InteractionUI == nullptr || !InteractionUI.GetFocusWidgetPosition(FocusPosition))
        {
            FocusIcon.SetVisibility(ESlateVisibility::Collapsed);
            return;
        }

        FocusIcon.SetRenderTranslation(FocusPosition - Slate::GetLocalSize(MyGeometry) * 0.5);
        FocusIcon.SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    UFUNCTION(BlueprintOverride)
    void OnOffersChanged(const TArray<FFVInteractionOfferData>&in Offers)
    {
        for (UInteractionSlotWidget OfferWidget : OfferPool)
        {
            if (IsValid(OfferWidget))
            {
                OfferWidget.Clear();
            }
        }

        OfferPool.Empty();
            
        if (Offers.IsEmpty())
        {
            return;
        }

        EnsureSlotPoolSize(Offers.Num());

        for (int i = 0; i < OfferPool.Num(); i++)
        {
            UInteractionSlotWidget OfferWidget = OfferPool[i];
            const FFVInteractionOfferData Offer = Offers[i];


            FInteractionSlotStyle Style; 
            InteractionSlotStyles.FindRow(Offer.ActionTag.GetTagName(), Style);
            
            // FInteractionKeyBinding Binding;
            // ResolveKeyBinding(Prompt.InputTag, Binding);   

            OfferWidget.SetSlotData(Style, Offer.bAvailable);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnCrosshairChanged(UTexture2D Icon, FGameplayTag InteractableType)
    {
        if (FocusIcon != nullptr && Icon != nullptr)
        {
            FocusIcon.SetBrushFromTexture(Icon);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnReticleVisibilityChanged(bool bVisible)
    {
        if (CenterReticle != nullptr)
        {
            CenterReticle.SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnOfferProgress(FGameplayTag ActionTag, float32 Progress)
    {
    }

    UFUNCTION(BlueprintOverride)
    void OnOfferEnded(FGameplayTag ActionTag, bool bSuccess)
    {
    }

    private void EnsureSlotPoolSize(int32 Size)
    {
        if (SlotFirstWidgetClass == nullptr || SlotMidWidgetClass == nullptr || SlotLastWidgetClass == nullptr)
        {
            Print("Slot widget class not set on interaction set.");
        }

        int i = 0;
        while (OfferPool.Num() < Size)
        {   
            UInteractionSlotWidget NewOffer;

            if (Size == 1)
            {
                NewOffer = WidgetBlueprint::CreateWidget(SlotMidWidgetClass, GetOwningPlayer());
            }
            else if (i == 0)
            {
                NewOffer = WidgetBlueprint::CreateWidget(SlotFirstWidgetClass, GetOwningPlayer());
            }
            else if (i == Size - 1)
            {
                NewOffer = WidgetBlueprint::CreateWidget(SlotLastWidgetClass, GetOwningPlayer());
            }
            else
            {
                NewOffer = WidgetBlueprint::CreateWidget(SlotMidWidgetClass, GetOwningPlayer());
            }

            InteractionSetBox.AddChild(NewOffer);
            OfferPool.Add(NewOffer);

            i++;
        }
    }
}