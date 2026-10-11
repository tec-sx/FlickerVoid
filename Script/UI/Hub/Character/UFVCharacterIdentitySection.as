// Character section about how the world sees the player: who they are, what they wear, whether they pass as someone
// else, fame, notoriety and titles. Reads the identity, equipment and FVSocialSystem; refreshes on fact and outfit changes.
UCLASS(Abstract)
class UFVCharacterIdentitySection : UFVHubMenuSectionWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Identity")
    FText NotDisguisedText;

    UPROPERTY(EditDefaultsOnly, Category = "Identity")
    FText NoTitlesText;

    UPROPERTY(EditDefaultsOnly, Category = "Identity")
    FText NothingWornText;

    UPROPERTY(EditDefaultsOnly, Category = "Identity", meta = (ToolTip = "Fame that fills the fame bar."))
    int FameScale = 100;

    UPROPERTY(EditDefaultsOnly, Category = "Identity", meta = (ToolTip = "Notoriety that fills the notoriety bar; the social system clamps it to 100."))
    int NotorietyScale = 100;

    UPROPERTY(BindWidgetOptional)
    UTextBlock NameText;

    UPROPERTY(BindWidgetOptional)
    UImage PortraitImage;

    UPROPERTY(BindWidgetOptional)
    UTextBlock LookText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DisguiseText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock FameText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar FameBar;

    UPROPERTY(BindWidgetOptional)
    UTextBlock NotorietyText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar NotorietyBar;

    UPROPERTY(BindWidgetOptional)
    UTextBlock TitlesText;

    private UFVEquipmentComponent Equipment;

    UFUNCTION(BlueprintOverride)
    void OnSectionInitialized()
    {
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.AddUFunction(this, n"HandleChanged");
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnRefresh()
    {
        BindEquipment();

        AActor Player = GetOwningPlayerPawn();
        UFVCharacterDefinition Definition = UFVIdentityComponent::GetActorDefinition(Player);

        if (NameText != nullptr)
        {
            NameText.SetText(Definition != nullptr ? Definition.Display.Name : FText());
        }
        if (PortraitImage != nullptr)
        {
            bool bHasPortrait = Definition != nullptr && !Definition.Display.Icon.IsNull();
            PortraitImage.SetVisibility(bHasPortrait ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
            if (bHasPortrait)
            {
                PortraitImage.SetBrushFromSoftTexture(Definition.Display.Icon, false);
            }
        }

        if (LookText != nullptr)
        {
            LookText.SetText(DescribeLook());
        }

        if (DisguiseText != nullptr)
        {
            UFVFactionDefinition Disguise = FVSocial::GetDisguiseFaction(Player);
            DisguiseText.SetText(Disguise != nullptr ? Disguise.Display.Name : NotDisguisedText);
        }

        int Fame = FVSocial::GetFame();
        int Notoriety = FVSocial::GetGlobalNotoriety();
        SetValue(FameText, FameBar, Fame, FameScale);
        SetValue(NotorietyText, NotorietyBar, Notoriety, NotorietyScale);

        if (TitlesText != nullptr)
        {
            TitlesText.SetText(DescribeTitles());
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnSectionHidden()
    {
        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.UnbindObject(this);
            Equipment = nullptr;
        }
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.UnbindObject(this);
        }
        OnSectionHidden();
    }

    private void BindEquipment()
    {
        UFVEquipmentComponent Current = UFVEquipmentComponent::Get(GetOwningPlayerPawn());
        if (Current == Equipment)
        {
            return;
        }
        OnSectionHidden();
        Equipment = Current;
        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
        }
    }

    private FText DescribeLook() const
    {
        FString Look;
        if (Equipment != nullptr)
        {
            for (FFVEquippedItem Entry : Equipment.GetAllEquipped())
            {
                if (Entry.Item == nullptr)
                {
                    continue;
                }
                FString ItemName = FVInventoryUI::GetItemName(Entry.Item).ToString();
                Look = Look.IsEmpty() ? ItemName : f"{Look}, {ItemName}";
            }
        }
        return Look.IsEmpty() ? NothingWornText : FText::FromString(Look);
    }

    private FText DescribeTitles() const
    {
        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        FString Titles;
        if (Social != nullptr)
        {
            for (UFVTitleDefinition Title : Social.GetHeldTitles())
            {
                FString TitleName = Title.Display.Name.ToString();
                Titles = Titles.IsEmpty() ? TitleName : f"{Titles}\n{TitleName}";
            }
        }
        return Titles.IsEmpty() ? NoTitlesText : FText::FromString(Titles);
    }

    private void SetValue(UTextBlock Text, UProgressBar Bar, int Value, int Scale)
    {
        if (Text != nullptr)
        {
            Text.SetText(FText::FromString(f"{Value}"));
        }
        if (Bar != nullptr)
        {
            Bar.SetPercent(float32(FVUIText::Fraction(float(Value), float(Scale))));
        }
    }

    UFUNCTION()
    private void HandleChanged()
    {
        if (IsVisible())
        {
            OnRefresh();
        }
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag SlotTag, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        HandleChanged();
    }
}
