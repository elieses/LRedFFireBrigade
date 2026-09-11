// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Backpack/UFFHorizontalMenuWidget.h"
#include "UI/Backpack/UFFItemCellWidget.h"
#include "Item/Equipment/UFFEquipmentComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"

void UFFHorizontalMenuWidget::ClearCells()
{
	// 解除旧绑定，避免重复订阅/悬挂引用
	if (BoundEquipment.IsValid())
	{
		BoundEquipment->OnEquipmentChanged.RemoveDynamic(this, &UFFHorizontalMenuWidget::RefreshCells);
	}
	BoundEquipment = nullptr;

	if (CellsBox)
	{
		CellsBox->ClearChildren();
	}
}

void UFFHorizontalMenuWidget::RefreshCells()
{
	if (BoundEquipment.IsValid())
	{
		RebuildFromEquipment(BoundEquipment.Get());
	}
}

void UFFHorizontalMenuWidget::RebuildFromEquipment(UFFEquipmentComponent* Equipment)
{
	ClearCells();
	if (!CellsBox || !Equipment)
	{
		return;
	}

	// 订阅装备组件变化：拾取/替换丢旧自动刷新
	BoundEquipment = Equipment;
	Equipment->OnEquipmentChanged.AddDynamic(this, &UFFHorizontalMenuWidget::RefreshCells);

	const TArray<FFFEquipmentEntry> Entries = Equipment->GetSlotEntries();
	for (const FFFEquipmentEntry& Entry : Entries)
	{
		if (!CellWidgetClass)
		{
			break;
		}

		UUserWidget* Cell = CreateWidget<UUserWidget>(this, CellWidgetClass);
		if (!Cell)
		{
			continue;
		}

		// 往横向容器加入单元格并均分宽度
		UHorizontalBoxSlot* HSlot = CellsBox->AddChildToHorizontalBox(Cell);
		if (HSlot)
		{
			FSlateChildSize ChildSize;
			ChildSize.SizeRule = ESlateSizeRule::Fill;
			ChildSize.Value = 1.f;
			HSlot->SetSize(ChildSize);
			HSlot->SetPadding(FMargin(2.f));
		}

		if (UFFItemCellWidget* ItemCell = Cast<UFFItemCellWidget>(Cell))
		{
			ItemCell->SetEntry(Entry);
		}
	}
}