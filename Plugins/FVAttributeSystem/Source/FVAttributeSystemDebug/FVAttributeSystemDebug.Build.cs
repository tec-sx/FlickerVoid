using UnrealBuildTool;

public class FVAttributeSystemDebug : ModuleRules
{
    public FVAttributeSystemDebug(ReadOnlyTargetRules Target) : base(Target)
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
            "FVAttributeSystem"
        });
    }
}
