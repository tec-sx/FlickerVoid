// Copyright Epic Games, Inc. All Rights Reserved.

#include "FVInteractionSystem.h"
#include "FVInteractionSystemStyle.h"
#include "FVInteractionSystemCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName FVInteractionSystemTabName("FVInteractionSystem");

#define LOCTEXT_NAMESPACE "FFVInteractionSystemModule"

void FFVInteractionSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FFVInteractionSystemStyle::Initialize();
	FFVInteractionSystemStyle::ReloadTextures();

	FFVInteractionSystemCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FFVInteractionSystemCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FFVInteractionSystemModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFVInteractionSystemModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FVInteractionSystemTabName, FOnSpawnTab::CreateRaw(this, &FFVInteractionSystemModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FFVInteractionSystemTabTitle", "FVInteractionSystem"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FFVInteractionSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FFVInteractionSystemStyle::Shutdown();

	FFVInteractionSystemCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FVInteractionSystemTabName);
}

TSharedRef<SDockTab> FFVInteractionSystemModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FFVInteractionSystemModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("FVInteractionSystem.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FFVInteractionSystemModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FVInteractionSystemTabName);
}

void FFVInteractionSystemModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FFVInteractionSystemCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FFVInteractionSystemCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFVInteractionSystemModule, FVInteractionSystem)
