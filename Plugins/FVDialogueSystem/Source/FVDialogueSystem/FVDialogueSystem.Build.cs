using UnrealBuildTool;

public class FVDialogueSystem : ModuleRules
{
    public FVDialogueSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "DeveloperSettings",
            "Flow",
            "FVCoreRuntime",
            "FVCoreFlow"
        });
    }
}
