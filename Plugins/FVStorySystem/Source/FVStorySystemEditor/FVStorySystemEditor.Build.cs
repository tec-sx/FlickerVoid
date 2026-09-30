using UnrealBuildTool;

public class FVStorySystemEditor : ModuleRules
{
    public FVStorySystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "InputCore",
            "GameplayTags",
            "FVCoreRuntime",
            "FVCoreEditor",
            "FVStorySystem"
        });
    }
}