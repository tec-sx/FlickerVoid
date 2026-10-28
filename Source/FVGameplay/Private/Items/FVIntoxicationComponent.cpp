#include "Items/FVIntoxicationComponent.h"

#include "Attributes/FVAttributeComponent.h"
#include "Attributes/FVAttributeDefinition.h"
#include "FVGameplayModule.h"
#include "GameFramework/Actor.h"
#include "Time/FVWorldClock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVIntoxicationComponent)

void UFVIntoxicationComponent::SleepItOff()
{
	if (Attributes && Intoxication)
	{
		Attributes->SetBaseValue(Intoxication, 0.f);
	}
}

void UFVIntoxicationComponent::BeginPlay()
{
	Super::BeginPlay();

	Attributes = UFVAttributeComponent::Find(GetOwner());
	if (!Attributes || !Intoxication)
	{
		UE_LOG(LogFVGameplay, Warning, TEXT("%s: intoxication needs an attribute component and its Intoxication attribute."), *GetNameSafe(GetOwner()));
		return;
	}

	Attributes->OnAttributeChanged.AddDynamic(this, &UFVIntoxicationComponent::HandleAttributeChanged);
	Peak = FMath::Max(Peak, Attributes->GetValue(Intoxication));

	Clock = UFVWorldClock::Get(this);
	if (Clock)
	{
		LastClockHour = GetClockHour();
		Clock->OnHourChanged.AddDynamic(this, &UFVIntoxicationComponent::HandleHourChanged);
	}
}

void UFVIntoxicationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Attributes)
	{
		Attributes->OnAttributeChanged.RemoveAll(this);
	}

	if (Clock)
	{
		Clock->OnHourChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UFVIntoxicationComponent::HandleHourChanged()
{
	const int32 ClockHour = GetClockHour();
	const int32 HoursPassed = ClockHour - LastClockHour;
	LastClockHour = ClockHour;

	if (HoursPassed > 0 && SoberPerHour > 0.f && Attributes->GetValue(Intoxication) > 0.f)
	{
		Attributes->ModifyBaseValue(Intoxication, -SoberPerHour * HoursPassed);
	}
}

void UFVIntoxicationComponent::HandleAttributeChanged(const UFVAttributeDefinition* Attribute, const float OldValue, const float NewValue)
{
	if (Attribute != Intoxication.Get())
	{
		return;
	}

	Peak = FMath::Max(Peak, NewValue);

	if (NewValue <= 0.f && OldValue > 0.f)
	{
		SoberUp();
	}
}

int32 UFVIntoxicationComponent::GetClockHour() const
{
	return Clock->GetDay() * 24 + FMath::FloorToInt32(Clock->GetHour());
}

void UFVIntoxicationComponent::SoberUp()
{
	const bool bHangover = Peak >= HangoverThreshold;
	Peak = 0.f;

	if (bHangover && Health && HangoverHealthLoss > 0.f)
	{
		Attributes->ModifyBaseValue(Health, -HangoverHealthLoss);
	}
}
