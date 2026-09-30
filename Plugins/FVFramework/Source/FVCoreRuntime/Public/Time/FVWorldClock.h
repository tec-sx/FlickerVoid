#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "FVWorldClock.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnClockEvent);

USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVDayPhase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time")
FGameplayTag Phase;

/** Phase starts at this hour (0-24). Phases must be sorted ascending. */
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta = (ClampMin = 0, ClampMax = 24))
float StartHour = 0.f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FlickerVoid World Clock"))
class FVCORERUNTIME_API UFVWorldClockSettings : public UDeveloperSettings
{
GENERATED_BODY()

public:
virtual FName GetCategoryName() const override { return TEXT("FlickerVoid"); }

/** Real seconds per in-game hour. 0 = time only advances via AdvanceTime. */
UPROPERTY(Config, EditAnywhere, Category = "Time", meta = (ClampMin = 0))
float SecondsPerHour = 0.f;

UPROPERTY(Config, EditAnywhere, Category = "Time", meta = (ClampMin = 0, ClampMax = 24))
float StartHour = 8.f;

UPROPERTY(Config, EditAnywhere, Category = "Time")
TArray<FFVDayPhase> Phases;

/** Fact storing total elapsed minutes, so time is saved with facts. */
UPROPERTY(Config, EditAnywhere, Category = "Time")
FGameplayTag MinutesFact;
};

UCLASS()
class FVCORERUNTIME_API UFVWorldClock : public UTickableWorldSubsystem
{
GENERATED_BODY()

public:
static UFVWorldClock* Get(const UObject* WorldContext);

virtual void OnWorldBeginPlay(UWorld& InWorld) override;
virtual void Tick(float DeltaTime) override;
virtual TStatId GetStatId() const override;
virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

UFUNCTION(BlueprintCallable, Category = "Time")
void AdvanceTime(float Hours);

UFUNCTION(BlueprintCallable, Category = "Time")
void SetPaused(bool bInPaused) { bPaused = bInPaused; }

UFUNCTION(BlueprintPure, Category = "Time")
int32 GetDay() const;

UFUNCTION(BlueprintPure, Category = "Time")
float GetHour() const;

UFUNCTION(BlueprintPure, Category = "Time")
FGameplayTag GetPhase() const { return CurrentPhase; }

UPROPERTY(BlueprintAssignable, Category = "Time")
FFVOnClockEvent OnHourChanged;

UPROPERTY(BlueprintAssignable, Category = "Time")
FFVOnClockEvent OnPhaseChanged;

UPROPERTY(BlueprintAssignable, Category = "Time")
FFVOnClockEvent OnDayChanged;

private:
void SetTotalMinutes(double NewMinutes);
FGameplayTag ResolvePhase(float Hour) const;

double TotalMinutes = 0.0;
FGameplayTag CurrentPhase;
bool bPaused = false;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Time of Day"))
struct FVCORERUNTIME_API FFVCondition_TimeOfDay : public FFVConditionBase
{
GENERATED_BODY()

/** If set, only the phase is checked. */
UPROPERTY(EditAnywhere, Category = "Condition")
FGameplayTag Phase;

UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "!Phase.IsValid()", ClampMin = 0, ClampMax = 24))
float FromHour = 0.f;

/** May be less than FromHour to wrap midnight. */
UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "!Phase.IsValid()", ClampMin = 0, ClampMax = 24))
float ToHour = 24.f;

virtual FText GetDescription() const override;

protected:
virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Advance Time"))
struct FVCORERUNTIME_API FFVEffect_AdvanceTime : public FFVEffectBase
{
GENERATED_BODY()

UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = 0))
float Hours = 1.f;

virtual void Apply(const FFVConditionContext& Context) const override;
virtual FText GetDescription() const override;
};