#pragma once

#include "CoreMinimal.h"
#include "EnhancedActionKeyMapping.h"
#include "InputCoreTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FVUILibrary.generated.h"

class APlayerController;
class UInputAction;
class UInputMappingContext;

/** Input queries for widgets: which keys trigger an Enhanced Input action, so screens can close or switch on rebindable keys. */
UCLASS()
class FVCOREUI_API UFVUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|UI", meta = (ToolTip = "Keys mapped to the action in the player's active mapping contexts."))
	static TArray<FKey> GetKeysForAction(const APlayerController* Player, const UInputAction* Action);

	UFUNCTION(BlueprintPure, Category = "FV|UI", meta = (ToolTip = "True if the key is mapped to the action in the player's active mapping contexts."))
	static bool IsKeyForAction(const APlayerController* Player, const UInputAction* Action, FKey Key);

	UFUNCTION(BlueprintPure, Category = "FV|UI", meta = (ToolTip = "Display name of the first key mapped to the action, or empty."))
	static FText GetActionKeyText(const APlayerController* Player, const UInputAction* Action);

	UFUNCTION(BlueprintPure, Category = "FV|UI", meta = (ToolTip = "The default key mappings of a mapping context, for a controls list."))
	static TArray<FEnhancedActionKeyMapping> GetContextMappings(const UInputMappingContext* Context);
};
