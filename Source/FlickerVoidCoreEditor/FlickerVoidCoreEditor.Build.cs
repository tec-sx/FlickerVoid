using UnrealBuildTool;

public class FlickerVoidCoreEditor : ModuleRules
{
    public FlickerVoidCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "FlickerVoidCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "GameplayTags",
                "DeveloperSettings",
                "UnrealEd",
                "ToolWidgets",
                "ToolMenus",
                "AssetDefinition",
                "ContentBrowser", 
            }
        );
        
        if (Target.bCompileAgainstEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "WorkspaceMenuStructure",
                    "SharedSettingsWidgets",
                }
            );
        }
    }
}