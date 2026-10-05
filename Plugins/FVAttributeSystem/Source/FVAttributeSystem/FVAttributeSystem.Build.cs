using UnrealBuildTool;

public class FVAttributeSystem : ModuleRules
{
    public FVAttributeSystem(ReadOnlyTargetRules Target) : base(Target)
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
