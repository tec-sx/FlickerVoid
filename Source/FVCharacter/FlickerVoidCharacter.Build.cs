using System.IO;
using UnrealBuildTool;

public class FlickerVoidCharacter : ModuleRules
{
    public FlickerVoidCharacter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayAbilities",
            "GameplayTags",
            "MotionTrajectory",
            "FlickerVoidCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ControlRig",
            "RigVM",
            "Flow",
            "AIModule"
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}