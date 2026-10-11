// Player options kept between sessions: what UGameUserSettings doesn't hold.
class UFVOptionsSave : USaveGame
{
    UPROPERTY()
    float MasterVolume = 1.0;

    UPROPERTY()
    bool bSubtitles = true;
}

// A choice in the quality list: its label and the scalability level it applies (0 Low to 4 Cinematic).
struct FFVQualityOption
{
    UPROPERTY(EditAnywhere, Category = "Options")
    FText Label;

    UPROPERTY(EditAnywhere, Category = "Options", meta = (ClampMin = 0, ClampMax = 4))
    int Level = 2;
}

// Options screen, pushed on UI.Layer.Menu from the main or pause menu. Three tabs laid out in SectionSwitcher:
// Gameplay (subtitles), Audio and Video (master volume, window mode, quality) and Controls (key list from the
// mapping contexts). Changes apply at once and are saved.
UCLASS(Abstract)
class UFVOptionsMenuWidget : UFVScreenWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Options")
    FText GameplayLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Options")
    FText AudioVideoLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Options")
    FText ControlsLabel;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Audio", meta = (ToolTip = "Sound mix pushed to apply the master volume to Master Sound Class."))
    USoundMix MasterSoundMix;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Audio")
    USoundClass MasterSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Video", meta = (ToolTip = "Labels for Fullscreen, Windowed Fullscreen and Windowed, in that order."))
    TArray<FText> WindowModeLabels;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Video", meta = (MaxEntries = 3, TitleProperty = "Label"))
    TArray<FFVQualityOption> QualityOptions;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Controls")
    TArray<UInputMappingContext> ControlContexts;

    UPROPERTY(EditDefaultsOnly, Category = "Options|Controls")
    TSubclassOf<UFVListEntryWidget> ControlEntryClass;

    UPROPERTY(EditDefaultsOnly, Category = "Options")
    FString SaveSlot = "FV_Options";

    UPROPERTY(BindWidget)
    UFVTabBarWidget SectionTabs;

    UPROPERTY(BindWidget)
    UWidgetSwitcher SectionSwitcher;

    UPROPERTY(BindWidgetOptional)
    UCheckBox SubtitlesCheck;

    UPROPERTY(BindWidgetOptional)
    USlider MasterVolumeSlider;

    UPROPERTY(BindWidgetOptional)
    UComboBoxString WindowModeCombo;

    UPROPERTY(BindWidgetOptional)
    UComboBoxString QualityCombo;

    UPROPERTY(BindWidgetOptional)
    UPanelWidget ControlsList;

    private UFVOptionsSave Options;
    private bool bApplyingUI = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        TArray<FText> Labels;
        Labels.Add(GameplayLabel);
        Labels.Add(AudioVideoLabel);
        Labels.Add(ControlsLabel);
        SectionTabs.OnTabSelected.AddUFunction(this, n"HandleSectionSelected");
        SectionTabs.SetTabs(Labels);

        if (SubtitlesCheck != nullptr)
        {
            SubtitlesCheck.OnCheckStateChanged.AddUFunction(this, n"HandleSubtitlesChanged");
        }
        if (MasterVolumeSlider != nullptr)
        {
            MasterVolumeSlider.OnValueChanged.AddUFunction(this, n"HandleVolumeChanged");
            MasterVolumeSlider.SetIsEnabled(MasterSoundMix != nullptr && MasterSoundClass != nullptr);
        }
        if (WindowModeCombo != nullptr)
        {
            WindowModeCombo.ClearOptions();
            for (FText Label : WindowModeLabels)
            {
                WindowModeCombo.AddOption(Label.ToString());
            }
            WindowModeCombo.OnSelectionChanged.AddUFunction(this, n"HandleWindowModeChanged");
        }
        if (QualityCombo != nullptr)
        {
            QualityCombo.ClearOptions();
            for (FFVQualityOption Option : QualityOptions)
            {
                QualityCombo.AddOption(Option.Label.ToString());
            }
            QualityCombo.OnSelectionChanged.AddUFunction(this, n"HandleQualityChanged");
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnActivated()
    {
        LoadOptions();
        SyncControls();
        BuildControlsList();
        if (SectionTabs.GetSelectedIndex() < 0)
        {
            SectionTabs.SelectTab(0);
        }
    }

    // Loads the saved options and applies them; call once at startup so they hold before the menu is opened.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void LoadOptions()
    {
        Options = FVOptions::Load(SaveSlot);
        ApplyAudio();
        Gameplay::SetSubtitlesEnabled(Options.bSubtitles);
    }

    private void SaveOptions()
    {
        if (Options != nullptr)
        {
            Gameplay::SaveGameToSlot(Options, SaveSlot, 0);
        }
    }

    private void ApplyAudio()
    {
        FVOptions::ApplyVolume(MasterSoundMix, MasterSoundClass, Options);
    }

    // Puts the current values into the controls without triggering their change handlers.
    private void SyncControls()
    {
        bApplyingUI = true;

        if (SubtitlesCheck != nullptr)
        {
            SubtitlesCheck.SetIsChecked(Options.bSubtitles);
        }
        if (MasterVolumeSlider != nullptr)
        {
            MasterVolumeSlider.SetValue(float32(Options.MasterVolume));
        }

        UGameUserSettings Settings = UGameUserSettings::GetGameUserSettings();
        if (WindowModeCombo != nullptr && Settings != nullptr)
        {
            WindowModeCombo.SetSelectedIndex(int(Settings.GetFullscreenMode()));
        }
        if (QualityCombo != nullptr && Settings != nullptr)
        {
            int Current = Settings.GetOverallScalabilityLevel();
            int Selected = -1;
            for (int i = 0; i < QualityOptions.Num(); i++)
            {
                if (QualityOptions[i].Level == Current)
                {
                    Selected = i;
                }
            }
            QualityCombo.SetSelectedIndex(Selected);
        }

        bApplyingUI = false;
    }

    private void BuildControlsList()
    {
        if (ControlsList == nullptr || !ControlEntryClass.IsValid())
        {
            return;
        }

        ControlsList.ClearChildren();
        int Index = 0;
        for (UInputMappingContext Context : ControlContexts)
        {
            for (FEnhancedActionKeyMapping Mapping : FVUI::GetContextMappings(Context))
            {
                if (Mapping.Action == nullptr)
                {
                    continue;
                }

                FFVListEntry Entry;
                Entry.Title = Mapping.Action.ActionDescription.IsEmpty() ? FText::FromString(Mapping.Action.GetName().ToString()) : Mapping.Action.ActionDescription;
                Entry.Value = Mapping.Key.GetDisplayName(false);

                UFVListEntryWidget Widget = WidgetBlueprint::CreateWidget(ControlEntryClass, GetOwningPlayer());
                Widget.SetEntry(Entry, Index);
                ControlsList.AddChild(Widget);
                Index++;
            }
        }
    }

    UFUNCTION()
    private void HandleSectionSelected(int Index)
    {
        SectionSwitcher.SetActiveWidgetIndex(Index);
    }

    UFUNCTION()
    private void HandleSubtitlesChanged(bool bIsChecked)
    {
        if (bApplyingUI || Options == nullptr)
        {
            return;
        }
        Options.bSubtitles = bIsChecked;
        Gameplay::SetSubtitlesEnabled(bIsChecked);
        SaveOptions();
    }

    UFUNCTION()
    private void HandleVolumeChanged(float32 Value)
    {
        if (bApplyingUI || Options == nullptr)
        {
            return;
        }
        Options.MasterVolume = Value;
        ApplyAudio();
        SaveOptions();
    }

    UFUNCTION()
    private void HandleWindowModeChanged(FString SelectedItem, ESelectInfo SelectionType)
    {
        UGameUserSettings Settings = UGameUserSettings::GetGameUserSettings();
        int Index = WindowModeCombo.GetSelectedIndex();
        if (bApplyingUI || Settings == nullptr || Index < 0 || Index > 2)
        {
            return;
        }
        Settings.SetFullscreenMode(EWindowMode(Index));
        Settings.ApplySettings(false);
        Settings.SaveSettings();
    }

    UFUNCTION()
    private void HandleQualityChanged(FString SelectedItem, ESelectInfo SelectionType)
    {
        UGameUserSettings Settings = UGameUserSettings::GetGameUserSettings();
        int Index = QualityCombo.GetSelectedIndex();
        if (bApplyingUI || Settings == nullptr || !QualityOptions.IsValidIndex(Index))
        {
            return;
        }
        Settings.SetOverallScalabilityLevel(QualityOptions[Index].Level);
        Settings.ApplySettings(false);
        Settings.SaveSettings();
    }
}

namespace FVOptions
{
    UFVOptionsSave Load(FString Slot)
    {
        UFVOptionsSave Options = Cast<UFVOptionsSave>(Gameplay::LoadGameFromSlot(Slot, 0));
        if (Options == nullptr)
        {
            Options = Cast<UFVOptionsSave>(Gameplay::CreateSaveGameObject(UFVOptionsSave));
        }
        return Options;
    }

    void ApplyVolume(USoundMix Mix, USoundClass SoundClass, UFVOptionsSave Options)
    {
        if (Mix == nullptr || SoundClass == nullptr || Options == nullptr)
        {
            return;
        }
        Gameplay::SetSoundMixClassOverride(Mix, SoundClass, float32(Options.MasterVolume), float32(1.0), float32(0.0), true);
        Gameplay::PushSoundMixModifier(Mix);
    }

    // Applies the saved options with the sound settings of an options screen class; HUDs call this at startup.
    void ApplySaved(TSubclassOf<UFVOptionsMenuWidget> OptionsClass)
    {
        UFVOptionsMenuWidget Defaults = OptionsClass.IsValid() ? Cast<UFVOptionsMenuWidget>(OptionsClass.Get().GetDefaultObject()) : nullptr;
        if (Defaults == nullptr)
        {
            return;
        }

        UFVOptionsSave Options = Load(Defaults.SaveSlot);
        ApplyVolume(Defaults.MasterSoundMix, Defaults.MasterSoundClass, Options);
        Gameplay::SetSubtitlesEnabled(Options.bSubtitles);
    }
}
