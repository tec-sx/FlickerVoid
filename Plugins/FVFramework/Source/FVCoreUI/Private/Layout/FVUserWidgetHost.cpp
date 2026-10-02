#include "Layout/FVUserWidgetHost.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVUserWidgetHost)

UFVUserWidgetHost::UFVUserWidgetHost(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
}

void UFVUserWidgetHost::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;
	}
}

void UFVUserWidgetHost::SetContent(UUserWidget* InContent)
{
	if (!Root || !InContent)
	{
		return;
	}

	Root->ClearChildren();
	Content = InContent;

	if (UOverlaySlot* OverlaySlot = Root->AddChildToOverlay(Content))
	{
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
	}
}
