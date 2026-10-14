#include "FVUILibrary.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVUILibrary)

TArray<FKey> UFVUILibrary::GetKeysForAction(const APlayerController* Player, const UInputAction* Action)
{
	const ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;
	const UEnhancedInputLocalPlayerSubsystem* Input = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	return Input && Action ? Input->QueryKeysMappedToAction(Action) : TArray<FKey>();
}

bool UFVUILibrary::IsKeyForAction(const APlayerController* Player, const UInputAction* Action, const FKey Key)
{
	return Key.IsValid() && GetKeysForAction(Player, Action).Contains(Key);
}

FText UFVUILibrary::GetActionKeyText(const APlayerController* Player, const UInputAction* Action)
{
	const TArray<FKey> Keys = GetKeysForAction(Player, Action);
	return Keys.IsEmpty() ? FText::GetEmpty() : Keys[0].GetDisplayName(false);
}

TArray<FEnhancedActionKeyMapping> UFVUILibrary::GetContextMappings(const UInputMappingContext* Context)
{
	return Context ? Context->GetMappings() : TArray<FEnhancedActionKeyMapping>();
}

TArray<UWidget*> UFVUILibrary::FindWidgetsOfClass(const UUserWidget* Owner, TSubclassOf<UWidget> WidgetClass)
{
	TArray<UWidget*> Result;
	if (Owner && Owner->WidgetTree && WidgetClass)
	{
		Owner->WidgetTree->ForEachWidget([&Result, WidgetClass](UWidget* Widget)
		{
			if (Widget && Widget->IsA(WidgetClass))
			{
				Result.Add(Widget);
			}
		});
	}
	return Result;
}

bool UFVUILibrary::IsWidgetInside(const UWidget* Widget, const UWidget* Container)
{
	const UWidget* Current = Widget;
	while (Current && Container)
	{
		if (Current == Container)
		{
			return true;
		}
		if (const UWidget* Parent = Current->GetParent())
		{
			Current = Parent;
		}
		else
		{
			// The root of a user widget's layout: go on from the user widget that holds it.
			Current = Current->GetTypedOuter<UUserWidget>();
		}
	}
	return false;
}
