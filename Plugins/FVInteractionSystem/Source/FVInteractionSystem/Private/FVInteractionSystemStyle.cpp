// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVInteractionSystemStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FFVInteractionSystemStyle::StyleInstance = nullptr;

void FFVInteractionSystemStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FFVInteractionSystemStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FFVInteractionSystemStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FVInteractionSystemStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FFVInteractionSystemStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("FVInteractionSystemStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("FVInteractionSystem")->GetBaseDir() / TEXT("Resources"));

	Style->Set("FVInteractionSystem.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FFVInteractionSystemStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FFVInteractionSystemStyle::Get()
{
	return *StyleInstance;
}
