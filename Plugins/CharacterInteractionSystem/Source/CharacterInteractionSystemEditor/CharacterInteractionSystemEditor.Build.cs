// Copyright 2023 Dmitriy Vergasov All Rights Reserved

using UnrealBuildTool;

public class CharacterInteractionSystemEditor : ModuleRules
{
    public CharacterInteractionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "Slate",
                "SlateCore",
                "CharacterInteractionSystem",
                "UnrealEd",
                "EditorStyle",
                "Blutility",
                "UMG",
                "UMGEditor",
                "PropertyEditor",
                "InputCore"
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
