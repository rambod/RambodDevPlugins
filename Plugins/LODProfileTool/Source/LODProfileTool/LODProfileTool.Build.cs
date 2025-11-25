using UnrealBuildTool;

public class LODProfileTool : ModuleRules
{
	public LODProfileTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// This module should only ever build for the Editor
		if (Target.bBuildEditor == false)
		{
			throw new BuildException("LODProfileTool is an editor-only plugin and cannot be built for non-editor targets.");
		}

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"DeveloperSettings"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",          // StaticMesh editing + Build()
				"ContentBrowser",    // Right-click menu extension
				"ToolMenus",         // Modern toolbar / menu API
				"AssetRegistry",     // Enumerate static meshes
				"Projects",          // Settings panel
				"InputCore",
				"LevelEditor"        // Toolbar or tab
			}
		);
	}
}
