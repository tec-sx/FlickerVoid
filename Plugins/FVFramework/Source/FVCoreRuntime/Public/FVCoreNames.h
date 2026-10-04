#pragma once

#include "CoreMinimal.h"

/** Names shared across FlickerVoid modules and plugins. Keep file-local names in their own .cpp. */
namespace FV::Names
{
	/** Project Settings category every FlickerVoid plugin's settings appear under. */
	inline const FName SettingsCategory = TEXT("FlickerVoid");

	/** Editor menu (main menu + toolbar dropdown) that plugin editor modules extend. */
	inline const FName EditorMenu = TEXT("LevelEditor.MainMenu.FlickerVoid");
}
