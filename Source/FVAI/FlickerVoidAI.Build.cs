using System.IO;
using UnrealBuildTool;

public class FlickerVoidAI : ModuleRules
{
    public FlickerVoidAI(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core",
            "CoreUObject",
            "Engine",
            "Mover",
            "GameplayAbilities",
            "GameplayTags",
            "AIModule",
            "FlickerVoidCore",
            "FlickerVoidWorld",
            "FlickerVoidCharacter"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}