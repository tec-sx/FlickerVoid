using System.IO;
using UnrealBuildTool;

public class FlickerVoidNarrative : ModuleRules
{
    public FlickerVoidNarrative(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
            "StateTreeModule",
            "GameplayStateTreeModule",
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "StateTreeEditorModule",
                "DeveloperSettings",
            });
        }
        
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}