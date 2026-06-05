using System.IO;
using UnrealBuildTool;

public class FlickerVoidGameplay : ModuleRules
{
	public FlickerVoidGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Engine",
			"GameFeatures",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"PhysicsCore",
			"FlickerVoidCore"
		});

        PrivateDependencyModuleNames.AddRange(new string[] {
            "UMG",
            "GameplayMessageRuntime",
        });

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
    }
}