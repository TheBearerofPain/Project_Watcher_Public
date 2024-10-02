// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Project_Watcher : ModuleRules
{
	public Project_Watcher(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem", "OnlineSubsystemUtils" });
		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
    }
}
