// HUD scanner readout: shown while scan mode is on. Names the focused scannable, fills the bar while scanning and
// lists the entries the player can read once the scan completes.
UCLASS(Abstract)
class UFVScannerReadoutWidget : UUserWidget
{
    UPROPERTY(BindWidgetOptional)
    UTextBlock TargetNameText;

    UPROPERTY(BindWidgetOptional)
    UProgressBar ScanProgressBar;

    UPROPERTY(BindWidgetOptional)
    UTextBlock EntriesText;

    private UFVScannerComponent Scanner;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.AddUFunction(this, n"HandlePawnChanged");
            BindScanner(Player.GetControlledPawn());
        }
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        APlayerController Player = GetOwningPlayer();
        if (Player != nullptr)
        {
            Player.OnPossessedPawnChanged.UnbindObject(this);
        }
        BindScanner(nullptr);
    }

    private void BindScanner(APawn Pawn)
    {
        if (Scanner != nullptr)
        {
            Scanner.OnScanModeChanged.UnbindObject(this);
            Scanner.OnFocusChanged.UnbindObject(this);
            Scanner.OnProgress.UnbindObject(this);
            Scanner.OnScanCompleted.UnbindObject(this);
        }

        Scanner = Pawn != nullptr ? UFVScannerComponent::Get(Pawn) : nullptr;
        if (Scanner != nullptr)
        {
            Scanner.OnScanModeChanged.AddUFunction(this, n"HandleScanModeChanged");
            Scanner.OnFocusChanged.AddUFunction(this, n"HandleFocusChanged");
            Scanner.OnProgress.AddUFunction(this, n"HandleProgress");
            Scanner.OnScanCompleted.AddUFunction(this, n"HandleScanCompleted");
        }

        HandleScanModeChanged(Scanner != nullptr && Scanner.IsScanModeActive());
        HandleFocusChanged(Scanner != nullptr ? Scanner.GetFocus() : nullptr);
    }

    UFUNCTION()
    private void HandlePawnChanged(APawn OldPawn, APawn NewPawn)
    {
        BindScanner(NewPawn);
    }

    UFUNCTION()
    private void HandleScanModeChanged(bool bActive)
    {
        SetVisibility(bActive ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }

    UFUNCTION()
    private void HandleFocusChanged(UFVScannableComponent Scannable)
    {
        UFVScanDefinition Definition = Scannable != nullptr ? Scannable.GetDefinition() : nullptr;
        if (TargetNameText != nullptr)
        {
            TargetNameText.SetText(Definition != nullptr ? Definition.Display.Name : FText());
        }

        if (Scannable != nullptr && Scannable.WasScanned())
        {
            ShowEntries(Scannable);
            HandleProgress(float32(1.0));
        }
        else
        {
            ShowEntries(nullptr);
            HandleProgress(float32(0.0));
        }
    }

    UFUNCTION()
    private void HandleProgress(float32 Progress)
    {
        if (ScanProgressBar != nullptr)
        {
            ScanProgressBar.SetPercent(Progress);
            ScanProgressBar.SetVisibility(Progress > 0.0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }
    }

    UFUNCTION()
    private void HandleScanCompleted(UFVScannableComponent Scannable)
    {
        ShowEntries(Scannable);
    }

    private void ShowEntries(UFVScannableComponent Scannable)
    {
        if (EntriesText == nullptr)
        {
            return;
        }

        FString Lines;
        if (Scannable != nullptr)
        {
            for (FFVScanEntry Entry : Scannable.GetVisibleEntries(GetOwningPlayerPawn()))
            {
                FString Line = Entry.Label.IsEmpty() ? Entry.Text.ToString() : f"{Entry.Label}: {Entry.Text}";
                Lines = Lines.IsEmpty() ? Line : f"{Lines}\n{Line}";
            }
        }

        EntriesText.SetText(FText::FromString(Lines));
        EntriesText.SetVisibility(Lines.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    }
}
