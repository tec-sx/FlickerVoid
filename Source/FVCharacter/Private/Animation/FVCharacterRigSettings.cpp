// Copyright FlickerVoid. All Rights Reserved.

#include "Animation/FVCharacterRigSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCharacterRigSettings)

UFVCharacterRigSettings::UFVCharacterRigSettings()
{
	SettingsId = NAME_None;
}

FPrimaryAssetId UFVCharacterRigSettings::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("FVCharacterRigSettings"), GetFName());
}
