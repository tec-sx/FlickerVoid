// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVInteractionSystemCommands.h"

#define LOCTEXT_NAMESPACE "FFVInteractionSystemModule"

void FFVInteractionSystemCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "FVInteractionSystem", "Bring up FVInteractionSystem window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
