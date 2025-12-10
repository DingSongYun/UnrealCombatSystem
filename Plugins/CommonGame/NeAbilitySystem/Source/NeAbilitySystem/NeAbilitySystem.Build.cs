// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NeAbilitySystem : ModuleRules
{
	public NeAbilitySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"GameplayAbilities",
				"NeGameFramework",
				"PropertyPath",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"GameplayTasks",
				"GameplayTags",
				"MotionWarping",
				"Niagara",
				"NetCore",
				"EnhancedInput",
				// ... add private dependencies that you statically link with here ...
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			OptimizeCode = CodeOptimization.Never;
		}

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"NiagaraEditor",
					"Sequencer",
				}
			);
		}	}
}
