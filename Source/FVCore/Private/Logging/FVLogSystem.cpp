// Copyright Epic Games, Inc. All Rights Reserved.

#include "Logging/FVLogSystem.h"
#include "Logging/FVLogCategories.h"
#include "Misc/OutputDeviceRedirector.h"

bool FFVLogSystem::bIsInitialized = false;

void FFVLogSystem::Initialize()
{
	if (bIsInitialized)
	{
		return;
	}

	UE_LOG(LogFVCore, Log, TEXT("FlickerVoid Logging System Initialized"));
	bIsInitialized = true;
}

void FFVLogSystem::Shutdown()
{
	if (!bIsInitialized)
	{
		return;
	}

	UE_LOG(LogFVCore, Log, TEXT("FlickerVoid Logging System Shutdown"));
	bIsInitialized = false;
}

void FFVLogSystem::HandleError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context)
{
	if (!Context.IsEmpty())
	{
		UE_LOG_REF(Category, Error, TEXT("[%s] %s"), *Context, *ErrorMessage);
	}
	else
	{
		UE_LOG_REF(Category, Error, TEXT("%s"), *ErrorMessage);
	}
}

void FFVLogSystem::HandleWarning(const FString& WarningMessage, const FLogCategoryBase& Category, const FString& Context)
{
	if (!Context.IsEmpty())
	{
		UE_LOG_REF(Category, Warning, TEXT("[%s] %s"), *Context, *WarningMessage);
	}
	else
	{
		UE_LOG_REF(Category, Warning, TEXT("%s"), *WarningMessage);
	}
}

void FFVLogSystem::HandleFatalError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context)
{
	if (!Context.IsEmpty())
	{
		UE_LOG_REF(Category, Fatal, TEXT("[%s] %s"), *Context, *ErrorMessage);
	}
	else
	{
		UE_LOG_REF(Category, Fatal, TEXT("%s"), *ErrorMessage);
	}
}

bool FFVLogSystem::CheckAndLogError(bool Condition, const FString& ErrorMessage, const FLogCategoryBase& Category)
{
	if (!Condition)
	{
		UE_LOG_REF(Category, Error, TEXT("Check Failed: %s"), *ErrorMessage);
		return false;
	}
	return true;
}

void FFVLogSystem::LogWithContext(const FString& Message, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Context)
{
	if (!Context.IsEmpty())
	{
		FMsg::Logf(__FILE__, __LINE__, Category.GetCategoryName(), Verbosity, TEXT("[%s] %s"), *Context, *Message);
	}
	else
	{
		FMsg::Logf(__FILE__, __LINE__, Category.GetCategoryName(), Verbosity, TEXT("%s"), *Message);
	}
}
