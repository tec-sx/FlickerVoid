class UFVInteractionPromptWidget : UUserWidget
{
	// Bind these in the UMG designer by naming the child widgets PrimarySlot,
	// SecondarySlot and HoldSlot (must be UFVInteractionSlotWidget instances).
	// Binding is by input slot, not by array position.
	UPROPERTY(BindWidget)
	UFVInteractionSlotWidget PrimarySlot;

	UPROPERTY(BindWidget)
	UFVInteractionSlotWidget SecondarySlot;

	UPROPERTY(BindWidget)
	UFVInteractionSlotWidget HoldSlot;

	// Optional radial/bar filled from 1 -> 0 while a timed offer counts down.
	UPROPERTY(BindWidgetOptional)
	UProgressBar TimeoutBar;

	private FGameplayMessageListenerHandle ListenerHandle;

	UFUNCTION(BlueprintOverride)
	void Construct()
	{
		ListenerHandle = UGameplayMessageSubsystem::Get().RegisterListener(
			GameplayTags::UI_Interaction_PromptChanged,
			this,
			n"OnPromptChanged",
			FFVUIInteractionPromptMessage());

		ResetSlots();
		SetVisibility(ESlateVisibility::Hidden);
	}

	UFUNCTION(BlueprintOverride)
	void Destruct()
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(ListenerHandle);
	}

	UFUNCTION()
	void OnPromptChanged(FGameplayTag Channel, const FFVUIInteractionPromptMessage& Message)
	{
		if (!Message.bVisible)
		{
			ResetSlots();
			SetVisibility(ESlateVisibility::Hidden);
			return;
		}

		for (int i = 0; i < Message.Slots.Num(); i++)
		{
			const FFVUIInteractionSlot Data = Message.Slots[i];
			UFVInteractionSlotWidget SlotWidget = GetSlotWidget(Data.Slot);

			if (SlotWidget != nullptr)
			{
				SlotWidget.SetSlotData(Data);
			}
		}

		if (TimeoutBar != nullptr)
		{
			TimeoutBar.SetVisibility(Message.bTimed
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
			TimeoutBar.SetPercent(Message.TimeRemainingRatio);
		}

		SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	private UFVInteractionSlotWidget GetSlotWidget(EFVInteractionSlot InSlot)
	{
		switch (InSlot)
		{
			case EFVInteractionSlot::Primary:
				return PrimarySlot;
			case EFVInteractionSlot::Secondary:
				return SecondarySlot;
			case EFVInteractionSlot::Hold:
				return HoldSlot;
		}
	}

	private void ResetSlots()
	{
		PrimarySlot.Clear();
		SecondarySlot.Clear();
		HoldSlot.Clear();
	}
}
