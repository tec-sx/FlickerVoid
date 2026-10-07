using UnrealBuildTool;

public class FVSocialSystemDebug : ModuleRules
{
    public FVSocialSystemDebug(ReadOnlyTargetRules Target) : base(Target)
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
            "FVSocialSystem"
        });
    }
}
