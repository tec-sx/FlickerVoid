#include "Layout/FVUILayout.h"

#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

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
return nullptr;
}

Layout = CreateWidget<UFVUILayout>(PC, LayoutClass);
Layout->AddToPlayerScreen();
return Layout;
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