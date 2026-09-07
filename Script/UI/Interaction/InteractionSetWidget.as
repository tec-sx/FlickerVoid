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

    private TArray<UInteractionSlotWidget> SlotPool;
    private TWeakObjectPtr<UInteractorComponent> Interactor;

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        UnbindFromInteractor();
    }

    UFUNCTION()
    void OnInteractionSetUpdated(TArray<FInteraction> Slots)
    {
        if (Slots.Num() == 0)
        {
            for (UInteractionSlotWidget SlotWidget : SlotPool)
            {
                SlotWidget.Clear();
            }

            SlotPool.Empty();
            return;
        }

        EnsureSlotPoolSize(Slots.Num());

        for (int i = 0; i < SlotPool.Num(); i++)
        {
            UInteractionSlotWidget SlotWidget = SlotPool[i];
            const FInteraction InteractionSlot = Slots[i];

            FInteractionSlotStyle Style; 
            InteractionSlotStyles.FindRow(InteractionSlot.ActionTag.GetTagName(), Style);

            // FInteractionKeyBinding Binding;
            // ResolveKeyBinding(Prompt.InputTag, Binding);   

            SlotWidget.SetSlotData(Style, InteractionSlot.bCanExecute);
        }
    }

    UFUNCTION()
    void BindToInteractor(UInteractorComponent InInteractor)
    {
    	UnbindFromInteractor();

    	Interactor = InInteractor;

    	if (IsValid(InInteractor))
    	{
    		InInteractor.OnOffersChanged.AddUFunction(this, n"HandleOffersChanged");
    		HandleOffersChanged(InInteractor.GetPrompts());
    	}
    }

    UFUNCTION()
    void UnbindFromInteractor()
    {
        UInteractorComponent Bound = Interactor.Get();

    	if (IsValid(Bound))
    	{
    		Bound.OnOffersChanged.Unbind(this, n"HandleOffersChanged");
    	}

    	Interactor = nullptr;
    }

    private void EnsureSlotPoolSize(int32 Size)
    {
        int i = 0;
        while (SlotPool.Num() < Size)
        {   
            UInteractionSlotWidget NewSlot;

            if (Size == 1)
            {
                NewSlot = WidgetBlueprint::CreateWidget(SlotMidWidgetClass, GetOwningPlayer());
            }
            else if (i == 0)
            {
                NewSlot = WidgetBlueprint::CreateWidget(SlotFirstWidgetClass, GetOwningPlayer());
            }
            else if (i == Size - 1)
            {
                NewSlot = WidgetBlueprint::CreateWidget(SlotLastWidgetClass, GetOwningPlayer());
            }
            else
            {
                NewSlot = WidgetBlueprint::CreateWidget(SlotMidWidgetClass, GetOwningPlayer());
            }

            InteractionSetBox.AddChild(NewSlot);
            SlotPool.Add(NewSlot);

            i++;
        }
    }

    UFUNCTION()
    private void HandleOffersChanged(const TArray<FInteraction>&in Slots)
    {
        SetVisibility(Slots.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    	OnInteractionSetUpdated(Slots);
    }
}