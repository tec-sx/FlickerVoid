using UnrealBuildTool;

public class FVCoreUI : ModuleRules
{
    public FVCoreUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "DeveloperSettings",
            "UMG",
            "CommonUI",
            "CommonInput",
            "EnhancedInput",
            "InputCore",
            "FVCoreRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore"
        });
    }
}
