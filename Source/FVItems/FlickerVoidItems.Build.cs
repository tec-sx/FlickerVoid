using System.IO;
using UnrealBuildTool;

public class FlickerVoidItems : ModuleRules
{
    public FlickerVoidItems(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "EnhancedInput",
            "FlickerVoidCore",
            "FlickerVoidGameplay",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "UMG",
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}