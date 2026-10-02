using System.IO;
using UnrealBuildTool;

public class FlickerVoidUI : ModuleRules
{
    public FlickerVoidUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "FlickerVoidCore",
            "CommonUI",
            "FVStorySystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "UMG",
            "Slate",
            "SlateCore",
            "GameplayMessageRuntime",
            "CommonInput",
            "DeveloperSettings"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}