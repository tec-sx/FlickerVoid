using System.IO;
using UnrealBuildTool;

public class FlickerVoidAI : ModuleRules
{
    public FlickerVoidAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayAbilities",
            "GameplayTags",
            "AIModule",
            "FlickerVoidCore",
            "FlickerVoidCharacter"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}