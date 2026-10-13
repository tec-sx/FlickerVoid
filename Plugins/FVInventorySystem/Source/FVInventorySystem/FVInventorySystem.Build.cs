using UnrealBuildTool;

public class FVInventorySystem : ModuleRules
{
    public FVInventorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "SlateCore",
            "UMG",
            "FVCoreRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "GameplayAbilities",
            "DeveloperSettings",
            "Projects",
            "InputCore",
            "Slate",
        });
    }
}
