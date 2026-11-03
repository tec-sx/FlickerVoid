using UnrealBuildTool;

public class FlickerVoidDebug : ModuleRules
{
	public FlickerVoidDebug(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"FVCoreRuntime",
			"FVCoreDebug",
			"FlickerVoidGameplay"
		});
	}
}
