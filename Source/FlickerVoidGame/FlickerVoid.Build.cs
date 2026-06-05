using System.IO;
using UnrealBuildTool;

public class FlickerVoid : ModuleRules
{
	public FlickerVoid(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"ApplicationCore",
			"Engine",
			"PhysicsCore",
			"StateTreeModule",
			"GameFeatures",
            "GameplayAbilities",
            "GameplayTasks",
			"GameplayTags",
            "GameplayStateTreeModule",
			"ModularGameplay",
            "AIModule",
			"EnhancedInput",
			"FlickerVoidCore",
			"FlickerVoidGameplay",
			"FlickerVoidItems",
			"FlickerVoidNarrative",
			"FlickerVoidCharacter",
			"FlickerVoidWorld",
			"FlickerVoidUI",
			"FlickerVoidAI"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
            "UMG",
			"RHI",
            "DeveloperSettings",
            "EngineSettings",
            "UAF",
            "UAFAnimGraph",      // UAF Animation Graph - provides trait interfaces
            "RigVM",
        });

		PublicIncludePaths.Add(ModuleDirectory);
		PrivateIncludePaths.Add(ModuleDirectory);
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UAFAnimGraphUncookedOnly" // Editor only module for UAF graph node templates
			});
		}

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

        SetupGameplayDebuggerSupport(Target);
    }
}
