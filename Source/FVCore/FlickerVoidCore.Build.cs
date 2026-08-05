using System.IO;
using UnrealBuildTool;

public class FlickerVoidCore : ModuleRules
{
    public FlickerVoidCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "ApplicationCore",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayTags",
            "PhysicsCore",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "Flow"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "StateTreeEditorModule", "AITestSuite",
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}