using System.IO;
using UnrealBuildTool;

public class FlickerVoidNarrative : ModuleRules
{
    public FlickerVoidNarrative(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "GameplayAbilities",
            "FlickerVoidCore",
            "FlickerVoidWorld",
            "FlickerVoidGameplay",
            "StateTreeModule"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}