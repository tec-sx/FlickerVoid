#pragma once

#include "CoreMinimal.h"

FVINTERACTIONSYSTEM_API DECLARE_LOG_CATEGORY_EXTERN(LogInteraction, Display, All);

void PrintString(const ELogVerbosity::Type Verbosity, const FString& Message, FLinearColor Color, float Duration);

#define LOG_INFO(Format, ...) \
{ \
FString FormattedMessage = FString::Printf(Format, ##__VA_ARGS__); \
PrintString(ELogVerbosity::Log, FormattedMessage, FLinearColor(0.0f, 1.0f, 0.0f), 5.0f); \
}

#define LOG_WARNING(Format, ...) \
{ \
FString FormattedMessage = FString::Printf(Format, ##__VA_ARGS__); \
PrintString(ELogVerbosity::Warning, FormattedMessage, FLinearColor(1.0f, 1.0f, 0.0f), 10.0f); \
}

#define LOG_ERROR(Format, ...) \
{ \
FString FormattedMessage = FString::Printf(Format, ##__VA_ARGS__); \
PrintString(ELogVerbosity::Error, FormattedMessage, FLinearColor(1.0f, 0.0f, 0.0f), 15.0f); \
}