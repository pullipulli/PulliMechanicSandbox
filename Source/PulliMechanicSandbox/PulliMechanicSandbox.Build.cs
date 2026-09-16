// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PulliMechanicSandbox : ModuleRules
{
	public PulliMechanicSandbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"PulliMechanicSandbox",
			"PulliMechanicSandbox/Public",
			"PulliMechanicSandbox/Variant_Combat",
			"PulliMechanicSandbox/Variant_Combat/AI",
			"PulliMechanicSandbox/Variant_Combat/Animation",
			"PulliMechanicSandbox/Variant_Combat/Gameplay",
			"PulliMechanicSandbox/Variant_Combat/Interfaces",
			"PulliMechanicSandbox/Variant_Combat/UI"
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			"PulliMechanicSandbox/Private",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
