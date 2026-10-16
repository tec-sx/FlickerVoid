// Goes on the player next to its attribute component: each game hour lowers the Intoxication attribute by Sober Per Hour. Getting sober (Intoxication 0) after a peak at or above Hangover Threshold costs Health once: the hangover. Alcohol and sobering items change Intoxication through their Usable effects (Modify Attribute).
class UFVIntoxicationComponent : UActorComponent
{
	UPROPERTY(EditAnywhere, Category = "Intoxication", meta = (ToolTip = "Raised by alcohol; lowered over time, by sleep and by sobering items."))
	UFVAttributeDefinition Intoxication;

	UPROPERTY(EditAnywhere, Category = "Intoxication", meta = (ToolTip = "Lowered by a hangover."))
	UFVAttributeDefinition Health;

	UPROPERTY(EditAnywhere, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Intoxication lost per game hour."))
	float SoberPerHour = 10.0;

	UPROPERTY(EditAnywhere, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Highest intoxication since the last sober moment from which getting sober gives a hangover."))
	float HangoverThreshold = 50.0;

	UPROPERTY(EditAnywhere, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Health lost to a hangover."))
	float HangoverHealthLoss = 10.0;

	UPROPERTY(SaveGame)
	float Peak = 0.0;

	UPROPERTY(SaveGame)
	int LastClockHour = -1;

	private UFVAttributeComponent Attributes;
	private UFVWorldClock Clock;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		Attributes = Cast<UFVAttributeComponent>(GetOwner().GetComponentByClass(UFVAttributeComponent));

		if (!IsValid(Attributes) || Intoxication == nullptr)
		{
			return;
		}

		Attributes.OnAttributeChanged.AddUFunction(this, n"HandleAttributeChanged");
		Peak = Math::Max(Peak, float(Attributes.GetValue(Intoxication)));

		Clock = UFVWorldClock::Get();

		if (Clock != nullptr)
		{
			LastClockHour = GetClockHour();
			Clock.OnHourChanged.AddUFunction(this, n"HandleHourChanged");
		}
	}

	UFUNCTION(BlueprintOverride)
	void EndPlay(EEndPlayReason EndPlayReason)
	{
		if (IsValid(Attributes))
		{
			Attributes.OnAttributeChanged.UnbindObject(this);
		}

		if (Clock != nullptr)
		{
			Clock.OnHourChanged.UnbindObject(this);
		}
	}

	UFUNCTION(meta = (ToolTip = "Ends intoxication at once, e.g. after a night's sleep; the hangover rule applies."))
	void SleepItOff()
	{
		if (IsValid(Attributes) && Intoxication != nullptr)
		{
			Attributes.SetBaseValue(Intoxication, 0.0);
		}
	}

	// Whole game hours since the clock started.
	private int GetClockHour()
	{
		return Clock.GetDay() * 24 + int(Clock.GetHour());
	}

	// The clock fires once per change, also when time jumps several hours: every hour passed counts.
	UFUNCTION()
	private void HandleHourChanged()
	{
		int ClockHour = GetClockHour();
		int HoursPassed = ClockHour - LastClockHour;
		LastClockHour = ClockHour;

		if (HoursPassed > 0 && SoberPerHour > 0.0 && Attributes.GetValue(Intoxication) > 0.0)
		{
			Attributes.ModifyBaseValue(Intoxication, -SoberPerHour * HoursPassed);
		}
	}

	UFUNCTION()
	private void HandleAttributeChanged(const UFVAttributeDefinition Attribute, float32 OldValue, float32 NewValue)
	{
		if (Attribute != Intoxication)
		{
			return;
		}

		Peak = Math::Max(Peak, float(NewValue));

		if (NewValue <= 0.0 && OldValue > 0.0)
		{
			SoberUp();
		}
	}

	private void SoberUp()
	{
		bool bHangover = Peak >= HangoverThreshold;
		Peak = 0.0;

		if (bHangover && Health != nullptr && HangoverHealthLoss > 0.0)
		{
			Attributes.ModifyBaseValue(Health, -HangoverHealthLoss);
		}
	}
}
