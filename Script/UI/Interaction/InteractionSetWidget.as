UCLASS(Abstract, Blueprintable)
class UInteractionSetWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UPanelWidget InteractionSetBox;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionOfferWidget> SlotFirstWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionOfferWidget> SlotMidWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration")
    TSubclassOf<UInteractionOfferWidget> SlotLastWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Configuration", Meta = (RequiredAssetDataTags = "RowStructure=/Script/Angelscript.InteractionSlotStyle"))
    UDataTable InteractionSlotStyles;

    private TArray<UInteractionOfferWidget> OfferPool;

    UFUNCTION()
    void OnPromptChanged(const TArray<FFVInteractionOffer>&in Offers)
    {
        if (Offers.Num() == 0)
        {
            for (UInteractionOfferWidget OfferWidget : OfferPool)
            {
                OfferWidget.Clear();
            }

            OfferPool.Empty();
            return;
        }

        EnsureSlotPoolSize(Offers.Num());

        for (int i = 0; i < OfferPool.Num(); i++)
        {
            UInteractionOfferWidget OfferWidget = OfferPool[i];
            const FFVInteractionOffer Offer = Offers[i];

            FInteractionSlotStyle Style; 
            InteractionSlotStyles.FindRow(Offer.ActionTag.GetTagName(), Style);

            // FInteractionKeyBinding Binding;
            // ResolveKeyBinding(Prompt.InputTag, Binding);   

            OfferWidget.SetSlotData(Offer, Style);
        }
    }

    UFUNCTION()
    private void OnPromptProgress(const FGameplayTag&in ActionTag, float32 Progress)
    {
    }

    UFUNCTION()
    void BindResponse(UFVInteractorResponseComponent_ShowPrompt ShowPromptResponse)
    {
    	if (IsValid(ShowPromptResponse))
    	{
    		ShowPromptResponse.OnPromptsChanged.AddUFunction(this, n"OnPromptChanged");
            ShowPromptResponse.OnPromptProgress.AddUFunction(this, n"OnPromptProgress");
            
            TArray<FFVInteractionOffer> Offers = ShowPromptResponse.GetPrompts();
            SetVisibility(Offers.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    		OnPromptChanged(Offers);
    	}
    }

    UFUNCTION()
    void UnbindResponse(UFVInteractorResponseComponent_ShowPrompt ShowPromptResponse)
    {
    	if (IsValid(ShowPromptResponse))
    	{
            ShowPromptResponse.OnPromptProgress.Unbind(this, n"OnPromptProgress");
    		ShowPromptResponse.OnPromptsChanged.Unbind(this, n"OnPromptChanged");
    	}
    }

    private void EnsureSlotPoolSize(int32 Size)
    {
        int i = 0;
        while (OfferPool.Num() < Size)
        {   
            UInteractionOfferWidget NewOffer;

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