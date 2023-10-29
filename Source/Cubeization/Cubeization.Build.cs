// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cubeization : ModuleRules
{
	public Cubeization(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"EnhancedInput"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// AI/MassAI Plugin Modules
			"MassAIBehavior",
			"MassAIDebug",

			// Runtime/MassEntity Plugin Modules
			"MassEntity",

			// Runtime/MassGameplay Plugin Modules
			"MassActors",
			"MassCommon",
			"MassGameplayDebug",
			"MassLOD",
			"MassMovement",
			"MassNavigation",
			"MassRepresentation",
			"MassReplication",
			"MassSpawner",
			"MassSimulation",
			"MassSignals",
		});
	}
}
