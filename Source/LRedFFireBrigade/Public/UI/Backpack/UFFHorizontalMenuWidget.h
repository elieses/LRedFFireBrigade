// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UFFHorizontalMenuWidget.generated.h"

class UHorizontalBox;
class UFFEquipmentComponent;
class UFFItemCellWidget;
class UUserWidget;

// 横列菜单（MVC 的 View）：根据装备组件的 MaxSlots 生成单元格，横向均匀分布。
// 订阅装备组件 OnEquipmentChanged —— 背包变化自动重建格子。
UCLASS()
class LREDFFIREBRIGADE_API UFFHorizontalMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 由背包根容器调用：读取装备条目并重建单元格（并订阅变更）
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void RebuildFromEquipment(UFFEquipmentComponent* Equipment);

	// 单元格的 widget 类（BP 里指定 WBP_ItemCell，本类用代码实例化）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> CellWidgetClass;

	// 单元格容器（横向 Box，代码里按 MaxSlots 均分）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UHorizontalBox> CellsBox;

	// 装备组件变化 → 自动重建
	UFUNCTION()
	void RefreshCells();

private:
	// 清空已有单元格并解除旧绑定
	void ClearCells();

	// 当前绑定的装备组件（自动刷新用）
	TWeakObjectPtr<UFFEquipmentComponent> BoundEquipment;
};