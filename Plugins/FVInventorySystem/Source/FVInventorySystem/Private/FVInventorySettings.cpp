#include "FVInventorySettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInventorySettings)

int32 UFVInventorySettings::EnforceWeightOverride = -1;
int32 UFVInventorySettings::EnforceSpaceOverride = -1;

namespace
{
	bool IsEnforced(const int32 Override, const bool bSetting)
	{
		if (Override < 0)
		{
			return bSetting;
		}
		return Override > 0;
	}
}

bool UFVInventorySettings::IsWeightEnforced() const
{
	return IsEnforced(EnforceWeightOverride, bEnforceWeight);
}

bool UFVInventorySettings::IsSpaceEnforced() const
{
	return IsEnforced(EnforceSpaceOverride, bEnforceSpace);
}
