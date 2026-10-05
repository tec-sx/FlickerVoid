using UnrealBuildTool;

public class FVInventorySystemEditor : ModuleRules
{
    public FVInventorySystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "GameplayTags",
            "DeveloperSettings",
            "UnrealEd",
            "ToolMenus",
            "FVCoreRuntime",
            "FVCoreEditor",
            "FVInventorySystem"
        });
    }
}
