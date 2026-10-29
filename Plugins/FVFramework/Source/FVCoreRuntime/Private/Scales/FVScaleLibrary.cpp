#include "Scales/FVScaleLibrary.h"

#include "Facts/FVFactDatabase.h"
#include "Facts/FVFactSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVScaleLibrary)

int32 UFVScaleLibrary::GetValue(const UObject* WorldContext, const UFVScaleDefinition* Scale)
{
	const UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	return Database && Scale ? Database->GetFact(Scale->Id) : 0;
}

bool UFVScaleLibrary::GetBand(const UObject* WorldContext, const UFVScaleDefinition* Scale, FFVScaleBand& OutBand)
{
	if (!Scale)
	{
		return false;
	}

	const int32 Value = GetValue(WorldContext, Scale);
	const FFVScaleBand* Band = Scale->FindBand(Value);
	if (!Band)
	{
		return false;
	}

	OutBand = *Band;
	return true;
}

bool UFVScaleLibrary::GetRange(const UFVScaleDefinition* Scale, int32& OutMin, int32& OutMax)
{
	const FFVFactDefinition* Fact = Scale ? GetDefault<UFVFactSettings>()->FindDefinition(Scale->Id) : nullptr;
	if (!Fact || !Fact->bClamp)
	{
		return false;
	}

	OutMin = Fact->Min;
	OutMax = Fact->Max;
	return true;
}

int32 UFVScaleLibrary::ModifyScale(UObject* WorldContext, const UFVScaleDefinition* Scale, const int32 Delta)
{
	if (!Scale || Delta == 0)
	{
		return 0;
	}

	// Only the change that applied is passed on, so a reward at the edge of the range earns nothing more elsewhere.
	const int32 Applied = AddClamped(WorldContext, Scale, Delta);
	if (Applied == 0)
	{
		return 0;
	}

	for (const FFVScaleLink& Link : Scale->Links)
	{
		if (Link.Scale && Link.Scale != Scale)
		{
			const int32 Change = Link.bMagnitude ? FMath::Abs(Applied) : Applied;
			const int32 Passed = FMath::RoundToInt(Change * Link.Factor);
			AddClamped(WorldContext, Link.Scale, Passed);
		}
	}

	return Applied;
}

int32 UFVScaleLibrary::AddClamped(UObject* WorldContext, const UFVScaleDefinition* Scale, const int32 Delta)
{
	UFVFactDatabase* Database = UFVFactDatabase::Get(WorldContext);
	if (!Database || !Scale->Id.IsValid() || Delta == 0)
	{
		return 0;
	}

	const int32 Before = Database->GetFact(Scale->Id);
	const int64 Wanted = static_cast<int64>(Before) + Delta;
	Database->SetFact(Scale->Id, static_cast<int32>(FMath::Clamp<int64>(Wanted, MIN_int32, MAX_int32)));

	const int32 After = Database->GetFact(Scale->Id);
	return After - Before;
}
