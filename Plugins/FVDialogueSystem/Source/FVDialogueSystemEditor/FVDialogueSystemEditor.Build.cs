using UnrealBuildTool;

public class FVDialogueSystemEditor : ModuleRules
{
    public FVDialogueSystemEditor(ReadOnlyTargetRules Target) : base(Target)
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
            "FVDialogueSystem"
        });
    }
}
