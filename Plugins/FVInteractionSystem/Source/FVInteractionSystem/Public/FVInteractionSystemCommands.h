// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "FVInteractionSystemStyle.h"

class FFVInteractionSystemCommands : public TCommands<FFVInteractionSystemCommands>
{
public:

	FFVInteractionSystemCommands()
		: TCommands<FFVInteractionSystemCommands>(TEXT("FVInteractionSystem"), NSLOCTEXT("Contexts", "FVInteractionSystem", "FVInteractionSystem Plugin"), NAME_None, FFVInteractionSystemStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};
