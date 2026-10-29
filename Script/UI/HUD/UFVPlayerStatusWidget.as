// HUD status: the faction the player passes as (hidden when not disguised), global notoriety and the latest title.
// Refreshes when facts or the outfit change.
UCLASS(Abstract)
class UFVPlayerStatusWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Status", meta = (ToolTip = "Notoriety that fills the bar; the social system clamps it to 100."))
    int NotorietyScale = 100;

    UPROPERTY(BindWidgetOptional)
    UWidget DisguisePanel;

    UPROPERTY(BindWidgetOptional)
    UTextBlock DisguiseText;

    UPROPERTY(BindWidgetOptional)
    UImage DisguiseIcon;

    UPROPERTY(BindWidgetOptional)
    UWidget NotorietyPanel;

    UPROPERTY(BindWidgetOptional)
    UTextBlock NotorietyText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar NotorietyBar;

    UPROPERTY(BindWidgetOptional)
    UTextBlock TitleText;

    private UFVEquipmentComponent Equipment;
    private UFVTitleDefinition LatestTitle;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.AddUFunction(this, n"HandlePawnChanged");
            BindEquipment(Player.GetControlledPawn());
        }

        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.AddUFunction(this, n"Refresh");
        }

        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            Social.OnTitleEarned.AddUFunction(this, n"HandleTitleEarned");
            TArray<UFVTitleDefinition> Held = Social.GetHeldTitles();
            LatestTitle = Held.Num() > 0 ? Held.Last() : nullptr;
        }

        Refresh();
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.UnbindObject(this);
        }
        BindEquipment(nullptr);

        UFVFactDatabase Facts = UFVFactDatabase::Get();
        if (Facts != nullptr)
        {
            Facts.OnFactsChanged.UnbindObject(this);
        }
        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            Social.OnTitleEarned.UnbindObject(this);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Refresh()
    {
        APawn Pawn = GetOwningPlayerPawn();

        UFVFactionDefinition Disguise = FVSocial::GetDisguiseFaction(Pawn);
        if (DisguisePanel != nullptr)
        {
            DisguisePanel.SetVisibility(Disguise != nullptr ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
        }
        if (DisguiseText != nullptr)
        {
            DisguiseText.SetText(Disguise != nullptr ? Disguise.Display.Name : FText());
        }
        if (DisguiseIcon != nullptr && Disguise != nullptr && !Disguise.Display.Icon.IsNull())
        {
            DisguiseIcon.SetBrushFromSoftTexture(Disguise.Display.Icon, false);
        }

        int Notoriety = FVSocial::GetGlobalNotoriety();
        if (NotorietyPanel != nullptr)
        {
            NotorietyPanel.SetVisibility(Notoriety > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
        }
        if (NotorietyText != nullptr)
        {
            NotorietyText.SetText(FText::FromString(f"{Notoriety}"));
        }
        if (NotorietyBar != nullptr)
        {
            NotorietyBar.SetPercent(float32(FVUIText::Fraction(float(Notoriety), float(NotorietyScale))));
        }

        if (LatestTitle != nullptr && !FVSocial::HasTitle(LatestTitle))
        {
            LatestTitle = nullptr;
        }
        if (TitleText != nullptr)
        {
            TitleText.SetText(LatestTitle != nullptr ? LatestTitle.Display.Name : FText());
            TitleText.SetVisibility(LatestTitle != nullptr ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }
    }

    private void BindEquipment(APawn Pawn)
    {
        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.UnbindObject(this);
        }
        Equipment = Pawn != nullptr ? UFVEquipmentComponent::Get(Pawn) : nullptr;
        if (Equipment != nullptr)
        {
            Equipment.OnEquipmentChanged.AddUFunction(this, n"HandleEquipmentChanged");
        }
    }

    UFUNCTION()
    private void HandlePawnChanged(APawn OldPawn, APawn NewPawn)
    {
        BindEquipment(NewPawn);
        Refresh();
    }

    UFUNCTION()
    private void HandleEquipmentChanged(FGameplayTag SlotTag, UFVItemDefinition OldItem, UFVItemDefinition NewItem)
    {
        Refresh();
    }

    UFUNCTION()
    private void HandleTitleEarned(const UFVTitleDefinition Title)
    {
        UFVSocialSubsystem Social = UFVSocialSubsystem::Get();
        if (Social != nullptr)
        {
            for (UFVTitleDefinition Held : Social.GetHeldTitles())
            {
                if (Held == Title)
                {
                    LatestTitle = Held;
                }
            }
        }
        Refresh();
    }
}
