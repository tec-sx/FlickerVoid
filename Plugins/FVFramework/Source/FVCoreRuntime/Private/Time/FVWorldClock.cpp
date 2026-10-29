#include "Time/FVWorldClock.h"

#include "Engine/World.h"
#include "Facts/FVFactDatabase.h"
#include "FVCoreRuntime.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVWorldClock)

#define LOCTEXT_NAMESPACE "FVWorldClock"

namespace FVClock
{
	constexpr double MinutesPerDay = 24.0 * 60.0;
}

UFVWorldClock* UFVWorldClock::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFVWorldClock>() : nullptr;
}

bool UFVWorldClock::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVWorldClock::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	ValidatePhases();

	const UFVWorldClockSettings* Settings = GetDefault<UFVWorldClockSettings>();
	const UFVFactDatabase* Facts = UFVFactDatabase::Get(&InWorld);
	const bool bRestore = Facts && Settings->MinutesFact.IsValid() && Facts->IsFactDefined(Settings->MinutesFact);

	TotalMinutes = bRestore ? Facts->GetFact(Settings->MinutesFact) : Settings->StartHour * 60.0;
	CurrentPhase = ResolvePhase(GetHour());
	WriteDayFact();
}

void UFVWorldClock::WriteDayFact() const
{
	const UFVWorldClockSettings* Settings = GetDefault<UFVWorldClockSettings>();
	UFVFactDatabase* Facts = UFVFactDatabase::Get(this);
	if (Facts && Settings->DayFact.IsValid())
	{
		Facts->SetFact(Settings->DayFact, GetDay());
	}
}

void UFVWorldClock::ValidatePhases()
{
	const TArray<FFVDayPhase>& Phases = GetDefault<UFVWorldClockSettings>()->Phases;
	TSet<FGameplayTag> Seen;
	for (int32 Index = 0; Index < Phases.Num(); ++Index)
	{
		const FFVDayPhase& Phase = Phases[Index];
		UE_CLOG(!Phase.Phase.IsValid(), LogFVCore, Warning, TEXT("World Clock phase %d has no tag."), Index);
		UE_CLOG(Phase.Phase.IsValid() && Seen.Contains(Phase.Phase), LogFVCore, Warning, TEXT("World Clock phase %s is listed twice."), *Phase.Phase.ToString());
		UE_CLOG(Index > 0 && Phase.StartHour <= Phases[Index - 1].StartHour, LogFVCore, Warning, TEXT("World Clock phase %d starts at or before the one above it; sort phases by start hour."), Index);
		Seen.Add(Phase.Phase);
	}
}

void UFVWorldClock::Tick(float DeltaTime)
{
	const float SecondsPerHour = GetDefault<UFVWorldClockSettings>()->SecondsPerHour;
	if (bPaused || SecondsPerHour <= 0.f)
	{
		return;
	}
	SetTotalMinutes(TotalMinutes + DeltaTime * 60.0 / SecondsPerHour);
}

TStatId UFVWorldClock::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFVWorldClock, STATGROUP_Tickables);
}

void UFVWorldClock::AdvanceTime(float Hours)
{
	if (Hours > 0.f)
	{
		SetTotalMinutes(TotalMinutes + Hours * 60.0);
	}
}

int32 UFVWorldClock::GetDay() const
{
	return FMath::FloorToInt32(TotalMinutes / FVClock::MinutesPerDay);
}

float UFVWorldClock::GetHour() const
{
	return static_cast<float>(FMath::Fmod(TotalMinutes, FVClock::MinutesPerDay) / 60.0);
}

void UFVWorldClock::SetTotalMinutes(double NewMinutes)
{
	const int32 OldHour = FMath::FloorToInt32(TotalMinutes / 60.0);
	const int32 OldDay = GetDay();
	TotalMinutes = NewMinutes;

	const UFVWorldClockSettings* Settings = GetDefault<UFVWorldClockSettings>();
	UFVFactDatabase* Facts = UFVFactDatabase::Get(this);
	if (Facts && Settings->MinutesFact.IsValid())
	{
		Facts->SetFact(Settings->MinutesFact, FMath::FloorToInt32(TotalMinutes));
	}

	if (FMath::FloorToInt32(TotalMinutes / 60.0) != OldHour)
	{
		OnHourChanged.Broadcast();
	}

	const FGameplayTag NewPhase = ResolvePhase(GetHour());
	if (NewPhase != CurrentPhase)
	{
		CurrentPhase = NewPhase;
		OnPhaseChanged.Broadcast();
	}

	if (GetDay() != OldDay)
	{
		WriteDayFact();
		OnDayChanged.Broadcast();
	}
}

FGameplayTag UFVWorldClock::ResolvePhase(float Hour) const
{
	const TArray<FFVDayPhase>& Phases = GetDefault<UFVWorldClockSettings>()->Phases;
	if (Phases.IsEmpty())
	{
		return FGameplayTag();
	}

	FGameplayTag Result = Phases.Last().Phase;
	for (const FFVDayPhase& Phase : Phases)
	{
		if (Hour >= Phase.StartHour)
		{
			Result = Phase.Phase;
		}
	}
	return Result;
}

bool FFVCondition_TimeOfDay::EvaluateImpl(const FFVConditionContext& Context) const
{
	const UFVWorldClock* Clock = UFVWorldClock::Get(Context.WorldContext);
	if (!Clock)
	{
		return false;
	}

	if (Phase.IsValid())
	{
		return Clock->GetPhase().MatchesTag(Phase);
	}

	const float Hour = Clock->GetHour();
	return FromHour <= ToHour ? (Hour >= FromHour && Hour < ToHour) : (Hour >= FromHour || Hour < ToHour);
}

FText FFVCondition_TimeOfDay::GetDescription() const
{
	if (Phase.IsValid())
	{
		return FText::Format(LOCTEXT("Phase", "During {0}"), FText::FromName(Phase.GetTagName()));
	}
	return FText::Format(LOCTEXT("Hours", "Between {0}h and {1}h"), FText::AsNumber(FromHour), FText::AsNumber(ToHour));
}

void FFVEffect_AdvanceTime::Apply(const FFVConditionContext& Context) const
{
	if (UFVWorldClock* Clock = UFVWorldClock::Get(Context.WorldContext))
	{
		Clock->AdvanceTime(Hours);
	}
}

FText FFVEffect_AdvanceTime::GetDescription() const
{
	return FText::Format(LOCTEXT("Advance", "Advance time {0}h"), FText::AsNumber(Hours));
}

#undef LOCTEXT_NAMESPACE