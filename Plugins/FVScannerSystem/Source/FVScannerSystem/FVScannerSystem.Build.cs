using UnrealBuildTool;

public class FVScannerSystem : ModuleRules
{
    public FVScannerSystem(ReadOnlyTargetRules Target) : base(Target)
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
