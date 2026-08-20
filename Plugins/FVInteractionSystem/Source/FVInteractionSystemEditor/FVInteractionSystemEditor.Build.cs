using UnrealBuildTool;

public class FVInteractionSystemEditor : ModuleRules
{
    public FVInteractionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "FVInteractionSystem",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "UnrealEd",
                "KismetCompiler",
                "Kismet",
                "Slate",
                "SlateCore",
            }
            );
    }
}
