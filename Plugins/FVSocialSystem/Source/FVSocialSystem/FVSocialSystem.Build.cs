using UnrealBuildTool;

public class FVSocialSystem : ModuleRules
{
    public FVSocialSystem(ReadOnlyTargetRules Target) : base(Target)
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

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "GameplayAbilities"
        });
    }
}
