// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * Global Error Handler for FlickerVoid
 * Provides centralized error handling and logging across all modules
 */
class FLICKERVOIDCORE_API FFVLogSystem
{
public:
	/**
	 * Initialize the global logging system
	 * Should be called during FVCore module startup
	 */
	static void Initialize();

	/**
	 * Shutdown the global logging system
	 * Should be called during FVCore module shutdown
	 */
	static void Shutdown();

	/**
	 * Global error handler with context
	 * @param ErrorMessage The error message to log
	 * @param Category The log category to use
	 * @param Context Additional context information (function name, line number, etc.)
	 */
	static void HandleError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));

	/**
	 * Global warning handler with context
	 * @param WarningMessage The warning message to log
	 * @param Category The log category to use
	 * @param Context Additional context information
	 */
	static void HandleWarning(const FString& WarningMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));

	/**
	 * Log a fatal error and terminate the application
	 * @param ErrorMessage The fatal error message
	 * @param Category The log category to use
	 * @param Context Additional context information
	 */
	static void HandleFatalError(const FString& ErrorMessage, const FLogCategoryBase& Category, const FString& Context = TEXT(""));

	/**
	 * Check a condition and log an error if false
	 * @param Condition The condition to check
	 * @param ErrorMessage The error message if condition is false
	 * @param Category The log category to use
	 * @return true if condition passed, false otherwise
	 */
	static bool CheckAndLogError(bool Condition, const FString& ErrorMessage, const FLogCategoryBase& Category);

	/**
	 * Log a message with formatted context
	 * @param Message The message to log
	 * @param Category The log category
	 * @param Verbosity The log verbosity level
	 * @param Context Additional context
	 */
	static void LogWithContext(const FString& Message, const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Context = TEXT(""));

private:
	static bool bIsInitialized;
};

/**
 * Helper macros for logging with automatic context
 */
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
