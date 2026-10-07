using UnrealBuildTool;

public class FVCoreDebug : ModuleRules
{
    public FVCoreDebug(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "AssetRegistry",
            "FVCoreRuntime"
        });
    }
}
