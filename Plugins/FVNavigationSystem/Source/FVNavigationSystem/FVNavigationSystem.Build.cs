using UnrealBuildTool;

public class FVNavigationSystem : ModuleRules
{
    public FVNavigationSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "DeveloperSettings",
            "FVCoreRuntime"
        });
    }
}
