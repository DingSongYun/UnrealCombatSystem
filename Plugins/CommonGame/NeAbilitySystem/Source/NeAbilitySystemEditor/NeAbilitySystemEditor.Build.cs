// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NeAbilitySystemEditor : ModuleRules
{
	public NeAbilitySystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		OptimizeCode = CodeOptimization.Never;
		
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
				"NeAbilitySystem",
				"NeGameFramework"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayAbilities",
				"AssetTools",
				"UnrealEd",
                "Kismet",
                "GameplayAbilitiesEditor",
                "NeEditorFramework",
                "SceneOutliner",
				"AdvancedPreviewScene",
				"InputCore",
				"CommonMenuExtensions",
				"ToolMenus",
				"LevelEditor",
				"SequencerWidgets",
				"GraphEditor",
				"BlueprintGraph",
				"EngineSettings",
				"Persona",
				"Niagara",
				"NiagaraEditor",
				"Sequencer",
				"PropertyPath",
				"GameplayTags",
				"GameplayTagsEditor",
                // ... add private dependencies that you statically link with here ...
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
