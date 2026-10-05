#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/FVDefinition.h"

class APawn;

namespace FVDebug
{
	FVCOREDEBUG_API APawn* GetPlayerPawn(const UWorld* World);

	/** Matches a gameplay tag by full name; logs a warning when it doesn't exist. */
	FVCOREDEBUG_API FGameplayTag FindTag(const FString& Name);

	/** Loads the first definition of Class whose Id tag or asset name matches IdOrName. */
	FVCOREDEBUG_API UFVDefinition* FindDefinition(const UClass* Class, const FString& IdOrName);

	template<typename T>
	T* FindDefinition(const FString& IdOrName)
	{
		return Cast<T>(FindDefinition(T::StaticClass(), IdOrName));
	}
}

DECLARE_LOG_CATEGORY_EXTERN(LogFVDebug, Log, All);
