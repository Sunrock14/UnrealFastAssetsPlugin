// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FastAssets : ModuleRules
{
	public FastAssets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
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
				"CoreUObject",
				"Engine",
				"InputCore"
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"AssetTools",
				"ContentBrowser",
				"ContentBrowserData",
				"LevelEditor",
				"DesktopPlatform",
				"AssetRegistry",
				"PropertyEditor",
				"EditorWidgets",
				"WorkspaceMenuStructure",
				"EditorScriptingUtilities",
				"DeveloperSettings",
				"ApplicationCore"
				"ApplicationCore"
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
