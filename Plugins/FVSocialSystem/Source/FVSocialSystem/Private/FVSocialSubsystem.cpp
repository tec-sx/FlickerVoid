#include "FVSocialSubsystem.h"

#include "FVFactionDefinition.h"
#include "FVSocialStatics.h"
#include "Time/FVWorldClock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSocialSubsystem)

bool UFVSocialSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UFVSocialSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (UFVWorldClock* Clock = UFVWorldClock::Get(this))
	{
		Clock->OnHourChanged.AddUniqueDynamic(this, &UFVSocialSubsystem::HandleHourChanged);
	}
}

void UFVSocialSubsystem::Deinitialize()
{
	if (UFVWorldClock* Clock = UFVWorldClock::Get(this))
	{
		Clock->OnHourChanged.RemoveDynamic(this, &UFVSocialSubsystem::HandleHourChanged);
	}
	Super::Deinitialize();
}

void UFVSocialSubsystem::HandleHourChanged()
{
	const UFVSocialSettings* Settings = GetDefault<UFVSocialSettings>();
	if (Settings->NotorietyDecayPerHour <= 0)
	{
		return;
	}

	for (const TSoftObjectPtr<UFVFactionDefinition>& Soft : Settings->Factions)
	{
		if (const UFVFactionDefinition* Faction = Soft.LoadSynchronous())
		{
			if (UFVSocialStatics::GetNotoriety(this, Faction) > 0)
			{
				UFVSocialStatics::ModifyNotoriety(this, Faction, -Settings->NotorietyDecayPerHour);
			}
		}
	}
}
