// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/Items/UFFItem.h"
#include "UFFItemCellWidget.generated.h"

class UImage;
class UBorder;

// 单个装备单元格（MVC 的 View）：直接操作绑定的图标控件显示物品，C++ 处理全部逻辑
UCLASS()
class LREDFFIREBRIGADE_API UFFItemCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 设置条目数据（由横列菜单调用），随即刷新图标显示
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SetEntry(const FFFEquipmentEntry& InEntry);

	UFUNCTION(BlueprintPure, Category = "Menu")
	const FFFEquipmentEntry& GetEntry() const { return Entry; }

	UFUNCTION(BlueprintPure, Category = "Menu")
	bool HasItem() const { return Entry.Item != nullptr; }

	// 图标控件（BP 里命名为 Icon）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> Icon;

	// 边框控件（BP 里命名为 Border；当前激活格子变红）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UBorder> Border;

protected:
	// 刷新显示：有物品 → 亮显图标；空槽 → 置灰半透明。激活状态 → 边框变红
	void RefreshVisual();

	// 首次使用时缓存 BP 里配的默认边框色，非激活时还原
	UPROPERTY(BlueprintReadOnly, Category = "Menu")
	FLinearColor DefaultBorderColor = FLinearColor(1.f, 1.f, 1.f, 0.f);

	bool bDefaultBorderCached = false;

	UPROPERTY(BlueprintReadOnly, Category = "Menu")
	FFFEquipmentEntry Entry;
};