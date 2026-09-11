// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Backpack/UFFItemCellWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UFFItemCellWidget::SetEntry(const FFFEquipmentEntry& InEntry)
{
	Entry = InEntry;
	RefreshVisual();
}

void UFFItemCellWidget::RefreshVisual()
{
	// 首次刷新时缓存 BP 里的默认边框色（激活高亮结束后还原用）
	if (!bDefaultBorderCached && Border)
	{
		DefaultBorderColor = Border->BrushColor;
		bDefaultBorderCached = true;
	}

	const bool bHasItem = HasItem();

	// 图标
	if (Icon)
	{
		if (bHasItem && Entry.Icon)
		{
			Icon->SetBrushFromTexture(Entry.Icon);
			Icon->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			Icon->SetBrushFromTexture(nullptr);
			Icon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 0.35f)); // 空槽：置灰半透明
		}
	}

	// 边框：当前激活（手持中）→ 红色；否则还原默认
	if (Border)
	{
		Border->SetBrushColor(Entry.bIsActive ? FLinearColor::Red : DefaultBorderColor);
	}
}