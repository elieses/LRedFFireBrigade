// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#pragma once

#include "Components/WidgetComponent.h"
#include "CoreMinimal.h"

#include "CISWidgetComponent.generated.h"

class UUserWidget;

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = "UserInterface", hidecategories = (Object, Activation, "Components|Activation", Sockets, Base, Lighting, LOD, Mesh), editinlinenew, meta = (BlueprintSpawnableComponent))
class CHARACTERINTERACTIONSYSTEM_API UCISWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Widget Class"), Category = "CIS|Widget")
	void K2_SetWidgetClass(TSubclassOf<UUserWidget> WidgetClassToSet);

	virtual void SetWidget(UUserWidget* InWidget) override;
	virtual void InitWidget() override;
};
