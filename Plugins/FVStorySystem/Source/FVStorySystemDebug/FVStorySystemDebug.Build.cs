using UnrealBuildTool;

public class FVStorySystemDebug : ModuleRules
{
    public FVStorySystemDebug(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "FVCoreRuntime",
            "FVCoreDebug",
            "FVStorySystem"
        });
    }
}
