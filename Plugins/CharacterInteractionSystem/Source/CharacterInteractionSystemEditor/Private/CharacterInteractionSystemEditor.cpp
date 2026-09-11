// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "CharacterInteractionSystemEditor.h"

#include "Customization/CISSceneComponentsOnActorCustomization.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FCharacterInteractionSystemEditorModule"

void FCharacterInteractionSystemEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.RegisterCustomPropertyTypeLayout("CISSceneComponentsOnActor", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCISSceneComponentsOnActorCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("CISPrimitiveComponentsOnActor", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FCISSceneComponentsOnActorCustomization::MakeInstance));
	}
}

void FCharacterInteractionSystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCharacterInteractionSystemEditorModule, CharacterInteractionSystemEditor)
