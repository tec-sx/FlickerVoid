#include "AssetDefinition_FactPreset.h"

#include "ContentBrowserMenuContexts.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

#include "FlickerVoidCoreEditor.h"
#include "FVCore/Public/FactDB/FVFactPreset.h"
#include "FactDB/Debugger/FVFactDebuggerStyle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_FactPreset)

namespace MenuExtentions_FactsPreset
{
	static FDelayedAutoRegisterHelper DelayedAutoRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]
		{
			FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);
			UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UFVFactPreset::StaticClass());

			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");

			{
				FToolUIAction UIAction;
				UIAction.ExecuteAction = FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& MenuContext)
				{
					if (const UContentBrowserAssetContextMenuContext* Context =
						UContentBrowserAssetContextMenuContext::FindContextWithAssets(MenuContext))
					{
						TArray<UFVFactPreset*> Presets = Context->LoadSelectedObjects<UFVFactPreset>();
						FFlickerVoidCoreEditorModule::Get().LoadFactPresets(Presets);
					}
				});
				UIAction.CanExecuteAction = FToolMenuCanExecuteAction::CreateLambda(
					[](const FToolMenuContext& MenuContext)
					{
						return FFlickerVoidCoreEditorModule::Get().IsGameInstanceStarted();
					});

				Section.AddMenuEntry(
					"FactPreset_Load",
					LOCTEXT("FactPreset_Load", "Load preset(s)"),
					LOCTEXT("FactPreset_LoadTooltip", "Load all facts from preset(s) (only in PIE)"),
					FSlateIcon(FFVFactDebuggerStyle::GetStyleSetName(), "ClassIcon.FactPreset"),
					UIAction
				);
			}
		}));
	});
}

FText UAssetDefinition_FactPreset::GetAssetDisplayName() const
{
	return LOCTEXT("FactPreset", "Fact Preset");
}

FLinearColor UAssetDefinition_FactPreset::GetAssetColor() const
{
	return FFVFactDebuggerStyle::Get().GetColor("Colors.FactPreset");
}

TSoftClassPtr<UObject> UAssetDefinition_FactPreset::GetAssetClass() const
{
	return UFVFactPreset::StaticClass();
}
