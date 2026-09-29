// Copyright 2024, Maksym Kapelianovych. Licensed under MIT license.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Types/SlateEnums.h"
#include "GameplayTagContainer.h"
#include "FVFactDebuggerSettingsLocal.generated.h"

USTRUCT()
struct FFVSearchToggleState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsToggleChecked = true;
	UPROPERTY()
	FText SearchText;
};


UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class UFVFactDebuggerSettingsLocal : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config)
	bool bShowRootFactTag = true;

	UPROPERTY(Config)
	bool bShowFullFactNames = true;

	UPROPERTY(Config)
	bool bShouldStackHierarchyHeaders = true;

	UPROPERTY(Config)
	bool bShowFavoritesInMainTree = false;

	UPROPERTY(Config)
	bool bShowOnlyLeafFacts = false;

	UPROPERTY(Config)
	bool bShowOnlyDefinedFacts = false;

	UPROPERTY(Config)
	TEnumAsByte<EOrientation> Orientation = Orient_Horizontal;

	UPROPERTY(Config, meta=(Categories="Fact"))
	TArray<FGameplayTag> FavoriteFacts;

	UPROPERTY(Config)
	TArray<FFVSearchToggleState> ToggleStates;
};
