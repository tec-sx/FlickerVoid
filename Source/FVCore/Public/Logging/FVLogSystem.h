// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

class FLICKERVOIDCORE_API FFVLogSystem
{
public:

	static void Initialize();
	static void Shutdown();
	
	static void HandleInfo(const FString& InfoMessage, const FLogCategoryBase& Category, const FString& Context);
	static void HandleError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));
	static void HandleWarning(const FString& WarningMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));
	static void HandleFatalError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));
	static bool CheckAndLogError(bool Condition, const FString& ErrorMessage, const FLogCategoryBase& Category);
	static void LogWithContext(const FString& Message, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Context = TEXT(""));

private:
	static bool bIsInitialized;
};

/**
 * Helper macros for logging with automatic context
 */
#define FV_LOG_INFO(Category, Format, ...) \
	FFVLogSystem::HandleInfo(FString::Printf(TEXT(Format), ##__VA_ARGS__), Category, FString::Printf(TEXT("%s:%d"), TEXT(__FUNCTION__), __LINE__))

#define FV_LOG_ERROR(Category, Format, ...) \
	FFVLogSystem::HandleError(FString::Printf(TEXT(Format), ##__VA_ARGS__), Category, FString::Printf(TEXT("%s:%d"), TEXT(__FUNCTION__), __LINE__))

#define FV_LOG_WARNING(Category, Format, ...) \
	FFVLogSystem::HandleWarning(FString::Printf(TEXT(Format), ##__VA_ARGS__), Category, FString::Printf(TEXT("%s:%d"), TEXT(__FUNCTION__), __LINE__))

#define FV_LOG_FATAL(Category, Format, ...) \
	FFVLogSystem::HandleFatalError(FString::Printf(TEXT(Format), ##__VA_ARGS__), Category, FString::Printf(TEXT("%s:%d"), TEXT(__FUNCTION__), __LINE__))

#define FV_CHECK(Condition, Category, Format, ...) \
	FFVLogSystem::CheckAndLogError(Condition, FString::Printf(TEXT(Format), ##__VA_ARGS__), Category)

#define FV_LOG(Category, Verbosity, Format, ...) \
	FFVLogSystem::LogWithContext(FString::Printf(TEXT(Format), ##__VA_ARGS__), Category, ELogVerbosity::Verbosity, FString::Printf(TEXT("%s:%d"), TEXT(__FUNCTION__), __LINE__))
