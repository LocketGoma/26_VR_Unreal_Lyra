// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VRSimulateRuntime : ModuleRules
{
	public VRSimulateRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"LyraGame",
				// Public GenericTeam dependency inherited from ALyraCharacter.
				"AIModule",
				"EnhancedInput",
				"InputCore",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"ShooterCoreRuntime"
			}
			);

		PrivateDependencyModuleNames.AddRange(new string[] { "PhysicsCore" });
	}
}
