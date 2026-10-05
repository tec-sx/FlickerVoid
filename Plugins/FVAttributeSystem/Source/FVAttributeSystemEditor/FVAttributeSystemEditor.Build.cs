using UnrealBuildTool;

public class FVAttributeSystemEditor : ModuleRules
{
    public FVAttributeSystemEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "FVAttributeSystem"
        });
    }
}
