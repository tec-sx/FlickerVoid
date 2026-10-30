using UnrealBuildTool;

public class FVAnimationSystem : ModuleRules
{
	public FVAnimationSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings",
			"ControlRig",
			"RigVM",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"FVCoreRuntime",
		});
	}
}
