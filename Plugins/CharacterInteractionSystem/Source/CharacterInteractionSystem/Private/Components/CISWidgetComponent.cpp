// Copyright 2023 Dmitriy Vergasov All Rights Reserved

#include "Components/CISWidgetComponent.h"

#include "Blueprint/UserWidget.h"

void UCISWidgetComponent::K2_SetWidgetClass(TSubclassOf<UUserWidget> WidgetClassToSet)
{
	SetWidgetClass(WidgetClassToSet);
}

void UCISWidgetComponent::SetWidget(UUserWidget* InWidget)
{
	Super::SetWidget(InWidget);

	if (GetWidget() != nullptr)
	{
		GetWidget()->Rename(nullptr, GetOwner());
	}
}
void UCISWidgetComponent::InitWidget()
{
	Super::InitWidget();

	if (GetWidget() != nullptr)
	{
		GetWidget()->Rename(nullptr, GetOwner());
	}
}
