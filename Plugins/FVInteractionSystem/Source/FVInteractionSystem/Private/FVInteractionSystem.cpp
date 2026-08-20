// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVInteractionSystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FFVInteractionSystemModule"

void FFVInteractionSystemModule::StartupModule()
{
}

void FFVInteractionSystemModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFVInteractionSystemModule, FVInteractionSystem)
