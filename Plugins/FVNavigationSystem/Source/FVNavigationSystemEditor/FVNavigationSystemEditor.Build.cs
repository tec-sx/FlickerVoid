using UnrealBuildTool;

public class FVNavigationSystemEditor : ModuleRules
{
    public FVNavigationSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AssetRegistry",
            "Core",
            "CoreUObject",
            "Engine",
            "RenderCore",
            "Slate",
            "SlateCore",
            "InputCore",
            "GameplayTags",
            "DeveloperSettings",
            "UnrealEd",
            "ToolMenus",
            "PropertyEditor",
            "WorkspaceMenuStructure",
            "FVCoreRuntime",
            "FVCoreEditor",
            "FVNavigationSystem"
        });
    }
}
