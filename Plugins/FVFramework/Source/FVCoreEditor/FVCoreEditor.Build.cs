using UnrealBuildTool;

public class FVCoreEditor : ModuleRules
{
    public FVCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "FVCoreRuntime"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UnrealEd",
            "PropertyEditor",
            "WorkspaceMenuStructure",
            "AssetRegistry",
            "DataValidation",
            "GameplayTags"
        });
    }
}
