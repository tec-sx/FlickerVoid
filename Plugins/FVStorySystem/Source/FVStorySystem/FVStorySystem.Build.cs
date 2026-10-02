using UnrealBuildTool;

public class FVStorySystem : ModuleRules
{
    public FVStorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "DeveloperSettings",
            "FVCoreRuntime",
            "Flow",
            "Yap",
            "GameplayMessageRuntime"
        });
    }
}