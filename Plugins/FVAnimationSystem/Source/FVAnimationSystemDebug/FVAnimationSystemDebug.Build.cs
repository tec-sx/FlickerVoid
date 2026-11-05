using UnrealBuildTool;

public class FVAnimationSystemDebug : ModuleRules
{
	public FVAnimationSystemDebug(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FVCoreRuntime",
			"FVCoreDebug",
			"FVAnimationSystem",
		});
	}
}
