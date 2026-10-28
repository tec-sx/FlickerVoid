using UnrealBuildTool;

public class FVAnimationSystemEditor : ModuleRules
{
	public FVAnimationSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"FVAnimationSystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings",
			"UnrealEd",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"FVCoreEditor",
		});
	}
}
