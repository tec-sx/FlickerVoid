class UFVInteractionPromptWidget : UUserWidget
{
    // Designer-configured: maps input slot tag → short key hint string shown in the slot
    // Example entries:
    //   InputTag.Interact.Primary       → "E"
    //   InputTag.Interact.PrimaryHold   → "[Hold] E"
    //   InputTag.Interact.Secondary     → "F"
    //   InputTag.Interact.SecondaryHold → "[Hold] F"
    UPROPERTY(EditDefaultsOnly, Category = "Interaction|Input Hints")
    TMap<FGameplayTag, FText> InputTagHints;

    // Four action slots — bind these in the UMG designer by naming the child
    // widgets Slot0, Slot1, Slot2, Slot3 (must be UFVInteractionSlotWidget instances).
    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot0;

    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot1;

    UPROPERTY(BindWidget)
    UFVInteractionSlotWidget Slot2;

    UPROPERTY()
    UFVInteractionSubsystem InteractionSubsystem;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        AFVPlayerController PC = Cast<AFVPlayerCharacterController>(GetOwningPlayer());

        if (IsValid(PC))
        {
            InteractionSubsystem = PC.GetInteractionSubsystem();
            PC.OnPossessedPawnChanged.AddUFunction(this, n"OnPawnChanged");
            
            if(IsValid(InteractionSubsystem))
            {
                OnPawnChanged(nullptr, PC.GetControlledPawn());
            }

        }

        SetVisibility(ESlateVisibility::Hidden);
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        UnbindFromInstigator();
        
        AFVPlayerController PC = Cast<AFVPlayerCharacterController>(GetOwningPlayer());

        if (IsValid(PC))
        {
            PC.OnPossessedPawnChanged.Unbind(this, n"OnPawnChanged");
        }
        
        InteractionSubsystem = nullptr;
    }

    
    UFUNCTION()
    private void OnPawnChanged(APawn OldPawn, APawn NewPawn)
    {
        UnbindFromInstigator();

        if (IsValid(NewPawn))
        {
            BindToInstigator();
        }
        else
        {
            OnFocusChanged(nullptr);
        }
    }

    UFUNCTION()
    void OnFocusChanged(UFVInteractionTargetComponent Target)
    {
        TArray<FFVInteractionActionInfo> Actions = InteractionSubsystem.GetAvailableActionsUIInfo();
        if (IsValid(Target) && Actions.Num() > 0)
        {
            SetVisibility(ESlateVisibility::HitTestInvisible);

                UpdateActionSlot(Slot0, Actions, 0);
                UpdateActionSlot(Slot1, Actions, 1);
                UpdateActionSlot(Slot2, Actions, 2);
        }
        else
        {
            SetVisibility(ESlateVisibility::Hidden);
            return;
        }
    }

    private void BindToInstigator()
    {
        InteractionSubsystem.OnFocusChanged.AddUFunction(this, n"OnFocusChanged");

        UFVInteractionTargetComponent Target = InteractionSubsystem.GetFocusedTarget();
        
        if (IsValid(Target))
        {
            OnFocusChanged(Target);
        }
    }

    private void UnbindFromInstigator()
    {
        if (IsValid(InteractionSubsystem))
        {
            InteractionSubsystem.OnFocusChanged.Unbind(this, n"OnFocusChanged");
        }
    }

    private void UpdateActionSlot(
		UFVInteractionSlotWidget ActionSlot,
		const TArray<FFVInteractionActionInfo>& Actions,
		int32 Index)
    {
        if (ActionSlot == nullptr)
        {
            return;
        }

        if (Index < Actions.Num())
        {
            FText Hint = GetHintForTag(Actions[Index].ActionTag);
            ActionSlot.SetSlotData(Actions[Index], Hint);
            ActionSlot.SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            ActionSlot.SetVisibility(ESlateVisibility::Hidden);
        }
    }

    private FText GetHintForTag(FGameplayTag Tag)
    {
        FText HintText;
        if (InputTagHints.Find(Tag, HintText))
        {
            return HintText;
        }
        return FText::FromString("?");
    }
}
