using UnrealBuildTool;

public class FVSocialSystemEditor : ModuleRules
{
    public FVSocialSystemEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "FVSocialSystem"
        });
    }
}
