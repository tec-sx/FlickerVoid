#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "FVUILayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;

UCLASS(Abstract, Blueprintable)
class FVCOREUI_API UFVUILayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|UI")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Container);

	UFUNCTION(BlueprintPure, Category = "FV|UI")
	UCommonActivatableWidgetContainerBase* GetLayer(FGameplayTag LayerTag) const;

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FV UI"))
class FVCOREUI_API UFVUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "UI")
	TSoftClassPtr<UFVUILayout> LayoutClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFVOnUIStackChanged);

UCLASS()
class FVCOREUI_API UFVUIManagerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|UI")
	UFVUILayout* EnsureLayout();

	UFUNCTION(BlueprintCallable, Category = "FV|UI", meta = (DeterminesOutputType = "WidgetClass"))
	UCommonActivatableWidget* PushScreen(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "FV|UI")
	void PopScreen(UCommonActivatableWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "FV|UI", meta = (DeterminesOutputType = "WidgetClass"))
	UUserWidget* PushUserWidget(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "FV|UI")
	void PopUserWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintPure, Category = "FV|UI")
	UFVUILayout* GetLayout() const { return Layout; }

	UPROPERTY(BlueprintAssignable, Category = "FV|UI")
	FFVOnUIStackChanged OnStackChanged;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFVUILayout> Layout;
};
