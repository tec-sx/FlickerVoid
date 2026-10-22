#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FVIntoxicationComponent.generated.h"

class UFVAttributeComponent;
class UFVAttributeDefinition;
class UFVWorldClock;

/**
 * Goes on the player next to its attribute component: each game hour lowers the Intoxication attribute by Sober Per
 * Hour. Getting sober (Intoxication 0) after a peak at or above Hangover Threshold costs Health once: the hangover.
 * Alcohol and sobering items change Intoxication through their Usable effects (Modify Attribute).
 */
UCLASS(ClassGroup = (FV), meta = (BlueprintSpawnableComponent))
class FLICKERVOIDGAMEPLAY_API UFVIntoxicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFVIntoxicationComponent() { PrimaryComponentTick.bCanEverTick = false; }

	UFUNCTION(BlueprintCallable, Category = "FV|Intoxication", meta = (ToolTip = "Ends intoxication at once, e.g. after a night's sleep; the hangover rule applies."))
	void SleepItOff();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The clock fires once per change, also when time jumps several hours: every hour passed counts. */
	UFUNCTION()
	void HandleHourChanged();

	UFUNCTION()
	void HandleAttributeChanged(const UFVAttributeDefinition* Attribute, float OldValue, float NewValue);

	/** Whole game hours since the clock started. */
	int32 GetClockHour() const;

	void SoberUp();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intoxication", meta = (ToolTip = "Raised by alcohol; lowered over time, by sleep and by sobering items."))
	TObjectPtr<UFVAttributeDefinition> Intoxication;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intoxication", meta = (ToolTip = "Lowered by a hangover."))
	TObjectPtr<UFVAttributeDefinition> Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Intoxication lost per game hour."))
	float SoberPerHour = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Highest intoxication since the last sober moment from which getting sober gives a hangover."))
	float HangoverThreshold = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Intoxication", meta = (ClampMin = 0, ToolTip = "Health lost to a hangover."))
	float HangoverHealthLoss = 10.f;

private:
	UPROPERTY(SaveGame)
	float Peak = 0.f;

	UPROPERTY(SaveGame)
	int32 LastClockHour = -1;

	UPROPERTY(Transient)
	TObjectPtr<UFVAttributeComponent> Attributes;

	UPROPERTY(Transient)
	TObjectPtr<UFVWorldClock> Clock;
};
