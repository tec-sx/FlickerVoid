#include "Layout/FVUILayout.h"

#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "FVCoreUI.h"
#include "HUD/FVHUD.h"
#include "Layout/FVUserWidgetHost.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVUILayout)

void UFVUILayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Container)
{
	if (!LayerTag.IsValid() || !Container)
	{
		return;
	}
	Layers.Add(LayerTag, Container);
}

UCommonActivatableWidgetContainerBase* UFVUILayout::GetLayer(FGameplayTag LayerTag) const
{
	const TObjectPtr<UCommonActivatableWidgetContainerBase>* Found = Layers.Find(LayerTag);
	return Found ? Found->Get() : nullptr;
}

UFVUILayout* UFVUIManagerSubsystem::EnsureLayout()
{
	if (Layout)
	{
		return Layout;
	}

	APlayerController* PC = GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
	UClass* LayoutClass = GetDefault<UFVUISettings>()->LayoutClass.LoadSynchronous();
	if (!PC || !LayoutClass)
	{
		if (PC && !LayoutClass)
		{
			UE_LOG(LogFVCoreUI, Warning, TEXT("LayoutClass is not set in Project Settings > FlickerVoid > UI."));
		}
		return nullptr;
	}

#if !UE_BUILD_SHIPPING
	if (GetDefault<UFVUISettings>()->bWarnIfHUDMissing && !Cast<AFVHUD>(PC->GetHUD()))
	{
		UE_LOG(LogFVCoreUI, Warning, TEXT("Layout created lazily; HUD is not an AFVHUD. Set the game mode HUD class to AFVHUD."));
	}
#endif

	Layout = CreateWidget<UFVUILayout>(PC, LayoutClass);
	Layout->AddToPlayerScreen();
	return Layout;
}

void UFVUIManagerSubsystem::ReleaseLayout()
{
	if (Layout)
	{
		Layout->RemoveFromParent();
		Layout = nullptr;
		OnStackChanged.Broadcast();
	}
}

UCommonActivatableWidget* UFVUIManagerSubsystem::PushScreen(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	UFVUILayout* CurrentLayout = EnsureLayout();
	UCommonActivatableWidgetContainerBase* Layer = CurrentLayout ? CurrentLayout->GetLayer(LayerTag) : nullptr;
	if (!Layer || !WidgetClass)
	{
		return nullptr;
	}

	UCommonActivatableWidget* Widget = Layer->AddWidget(WidgetClass);
	OnStackChanged.Broadcast();
	return Widget;
}

void UFVUIManagerSubsystem::PopScreen(UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		return;
	}
	Widget->DeactivateWidget();
	OnStackChanged.Broadcast();
}

UUserWidget* UFVUIManagerSubsystem::PushUserWidget(FGameplayTag LayerTag, TSubclassOf<UUserWidget> WidgetClass)
{
	UFVUILayout* CurrentLayout = EnsureLayout();
	UCommonActivatableWidgetContainerBase* Layer = CurrentLayout ? CurrentLayout->GetLayer(LayerTag) : nullptr;
	if (!Layer || !WidgetClass)
	{
		return nullptr;
	}

	UFVUserWidgetHost* Host = Cast<UFVUserWidgetHost>(Layer->AddWidget(UFVUserWidgetHost::StaticClass()));
	if (!Host)
	{
		return nullptr;
	}

	UUserWidget* Content = CreateWidget<UUserWidget>(CurrentLayout->GetOwningPlayer(), WidgetClass);
	Host->SetContent(Content);
	OnStackChanged.Broadcast();
	return Content;
}

void UFVUIManagerSubsystem::PopUserWidget(UUserWidget* Widget)
{
	UFVUserWidgetHost* Host = Widget ? Widget->GetTypedOuter<UFVUserWidgetHost>() : nullptr;
	if (!Host && Widget && Widget->GetParent())
	{
		Host = Widget->GetParent()->GetTypedOuter<UFVUserWidgetHost>();
	}
	PopScreen(Host);
}
