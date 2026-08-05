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
	APlayerCharacter Player;

	UFUNCTION(BlueprintOverride)
	void Construct()
	{
		Player = Cast<APlayerCharacter>(GetOwningPlayer().GetControlledPawn());
		
		if (Player != nullptr)
		{
			Player.InteractionInstigator.OnFocusChanged.AddUFunction(this, n"OnFocusChanged");
			OnFocusChanged(Player.InteractionInstigator.GetFocusedTarget());
		}

		SetVisibility(ESlateVisibility::Hidden);
	}

	UFUNCTION(BlueprintOverride)
	void Destruct()
	{
		if (Player != nullptr)
		{
			Player.InteractionInstigator.OnFocusChanged.Unbind(this, n"OnFocusChanged");
		}

		Player = nullptr;
	}

	UFUNCTION()
	void OnFocusChanged(UFVInteractionTargetComponent Target)
	{
		ResetSlots();
		SetVisibility(ESlateVisibility::Hidden);

		if (Player == nullptr || Target == nullptr)
		{
			return;
		}

		TArray<UFVInteractionAction> Actions = Target.GetAvailableActions();

		if (Actions.Num() > 0)
		{
			for (int i = 0; i < Actions.Num(); i++)
			{
				FGameplayTagContainer CharacterTags;
				Player.GetOwnedGameplayTags(CharacterTags);
				const FFVInteractionActionInfo Action = Actions[i].CreateActionUIInfo(CharacterTags);
			    UpdateActionSlot(Action, i);
			}

			SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	private void UpdateActionSlot(const FFVInteractionActionInfo Action, int32 Index)
	{
		UFVInteractionSlotWidget ActionSlot = GetSlotByIndex(Index);

		if (ActionSlot == nullptr)
		{
			return;
		}

        FText Hint = GetHintForTag(Action.ActionTag);

        ActionSlot.SetSlotData(Action, Hint);
        ActionSlot.SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	private UFVInteractionSlotWidget GetSlotByIndex(int32 Index)
	{
		switch (Index)
		{
			case 0:
				return Slot0;
			case 1:
				return Slot1;
			case 2:
				return Slot2;
			default:
				return nullptr;
		}
	}

	private void ResetSlots()
	{
		Slot0.SetVisibility(ESlateVisibility::Hidden);
		Slot1.SetVisibility(ESlateVisibility::Hidden);
		Slot2.SetVisibility(ESlateVisibility::Hidden);
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
