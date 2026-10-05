using UnrealBuildTool;

public class FVDialogueSystemDebug : ModuleRules
{
    public FVDialogueSystemDebug(ReadOnlyTargetRules Target) : base(Target)
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
            "FVDialogueSystem"
        });
    }
}
