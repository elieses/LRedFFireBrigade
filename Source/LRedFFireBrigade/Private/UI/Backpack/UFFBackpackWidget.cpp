// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Backpack/UFFBackpackWidget.h"
#include "UI/Backpack/UFFHorizontalMenuWidget.h"
#include "UI/Status/UFFCharacterStatusWidget.h"
#include "Item/Equipment/UFFEquipmentComponent.h"
#include "Characters/Firefighters/UFFCharacterStatsComponent.h"
#include "Characters/Firefighters/UFFCharacterData.h"

void UFFBackpackWidget::BindEquipment(UFFEquipmentComponent* Equipment)
{
	BoundEquipment = Equipment;

	if (HorizontalMenu)
	{
		HorizontalMenu->RebuildFromEquipment(Equipment);
	}
}

void UFFBackpackWidget::BindStats(UFFCharacterStatsComponent* Stats)
{
	if (StatusWidget)
	{
		StatusWidget->BindStats(Stats);
	}
}

void UFFBackpackWidget::BindCharacterData(UFFCharacterData* CharacterData)
{
	if (StatusWidget)
	{
		StatusWidget->BindCharacterData(CharacterData);
	}
}