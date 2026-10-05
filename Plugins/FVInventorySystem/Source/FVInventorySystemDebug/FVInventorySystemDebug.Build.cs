using UnrealBuildTool;

public class FVInventorySystemDebug : ModuleRules
{
    public FVInventorySystemDebug(ReadOnlyTargetRules Target) : base(Target)
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
            "FVInventorySystem"
        });
    }
}
