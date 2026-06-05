using System.IO;
using UnrealBuildTool;

public class FlickerVoidCharacter : ModuleRules
{
    public FlickerVoidCharacter(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayAbilities",
            "GameplayTags",
            "PoseSearch",
            "MotionTrajectory",
            "FlickerVoidCore",
            "FlickerVoidGameplay"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ControlRig",
            "RigVM",
            "UAF",
            "UAFAnimGraph",
            "StructUtils"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}