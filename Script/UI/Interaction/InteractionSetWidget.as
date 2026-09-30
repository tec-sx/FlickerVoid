UCLASS(Abstract, Blueprintable)
class UInteractionSetWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UPanelWidget InteractionSetBox;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotFirstWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotMidWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionSlotWidget> SlotLastWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration", Meta = (RequiredAssetDataTags = "RowStructure=/Script/Angelscript.InteractionSlotStyle"))
    UDataTable InteractionSlotStyles;


    private UFVInteractorComponent Interactor;
    private TArray<UInteractionSlotWidget> OfferPool;

    void Init(UFVInteractorComponent InInteractor)
    {
        Interactor = InInteractor;
        Interactor.OffersChanged.AddUFunction(this, n"OnOffersChanged");
        Interactor.InteractionCommitProgressed.AddUFunction(this, n"OnProgress");
        Interactor.InteractionCommitEnded.AddUFunction(this, n"OnEnded");

        OfferPool.Empty();
    }
    
    UFUNCTION()
    private void OnOffersChanged(const TArray<FFVInteractionOffer>&in Offers)
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
            const FFVInteractionOffer Offer = Offers[i];


            FInteractionSlotStyle Style; 
            InteractionSlotStyles.FindRow(Offer.ActionTag.GetTagName(), Style);
            
            // FInteractionKeyBinding Binding;
            // ResolveKeyBinding(Prompt.InputTag, Binding);   

            OfferWidget.SetSlotData(Style, Offer.bRequirementsMet);
        }
    }

    UFUNCTION()
    private void OnProgress(const FFVInteractionCommit&in Commit, float32 Progress)
    {
    }

    UFUNCTION()
    private void OnEnded(const FFVInteractionCommit&in Commit, bool bSuccess)
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