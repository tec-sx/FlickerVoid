// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

FVINTERACTIONSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogFVInteraction, Log, All);

class FFVInteractionSystemModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
