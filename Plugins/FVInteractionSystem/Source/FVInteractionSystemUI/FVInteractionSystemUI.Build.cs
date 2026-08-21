// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FVInteractionSystemUI : ModuleRules
{
    public FVInteractionSystemUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GameplayTags",
                "FVInteractionSystem",
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "UMG",
                "DeveloperSettings",
            }
            );
    }
}
