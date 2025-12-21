// Copyright NetEase Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealCombatSystem : ModuleRules
{
	public UnrealCombatSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Unleash the power of our plugins
		PublicDependencyModuleNames.AddRange(new string[] { 
			"NeAbilitySystem", 
			"ModularGameplayActors",
			"CommonLoadingScreen",
			"AsyncMixin"
		});
	}
}