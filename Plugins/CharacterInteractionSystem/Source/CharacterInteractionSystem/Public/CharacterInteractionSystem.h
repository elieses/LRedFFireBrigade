// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(CISLog, Warning, All);

class FCharacterInteractionSystemModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void CISMessage(const FString& Text, float Time = 5.f, const FColor& Color = FColor::Yellow, bool bShowOnTop = true);
