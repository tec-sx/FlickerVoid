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
			"AnimGraph",
			"DataValidation",
			"DeveloperSettings",
			"InputCore",
			"Persona",
			"PropertyEditor",
			"SkeletalMeshEditor",
			"SkeletonEditor",
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd",
			"FVCoreEditor",
		});
	}
}
