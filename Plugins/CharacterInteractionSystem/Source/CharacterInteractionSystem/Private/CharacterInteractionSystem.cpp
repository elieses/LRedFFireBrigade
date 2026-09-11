// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "CharacterInteractionSystem.h"

#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "FCharacterInteractionSystemModule"

DEFINE_LOG_CATEGORY(CISLog);

void FCharacterInteractionSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCharacterInteractionSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCharacterInteractionSystemModule, CharacterInteractionSystem)

void CISMessage(const FString& Text, float Time /*= 5.f*/, const FColor& Color /*= FColor::Yellow*/, bool bShowOnTop /*= true*/)
{
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(-1, Time, Color, Text, bShowOnTop);
#endif

	UE_LOG(CISLog, Display, TEXT("%s"), *Text);
}
