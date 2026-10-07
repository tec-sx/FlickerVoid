using UnrealBuildTool;

public class FVCoreFlow : ModuleRules
{
    public FVCoreFlow(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "Flow",
            "FVCoreRuntime"
        });
    }
}
