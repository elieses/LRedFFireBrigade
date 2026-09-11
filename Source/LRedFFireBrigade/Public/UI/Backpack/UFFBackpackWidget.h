// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UFFBackpackWidget.generated.h"

class UFFHorizontalMenuWidget;
class UFFEquipmentComponent;
class UFFCharacterStatsComponent;
class UFFCharacterStatusWidget;
class UFFCharacterData;

// 背包根容器（MVC 的 View 顶层）：几乎空白，装载"横列菜单"+ "状态条"，并转发装备/状态数据
UCLASS()
class LREDFFIREBRIGADE_API UFFBackpackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 由 Controller(PlayerController) 在选中角色变化时调用：把装备数据喂给菜单
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void BindEquipment(UFFEquipmentComponent* Equipment);

	// 由 Controller 在选中角色变化时调用：把状态数据喂给状态条
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void BindStats(UFFCharacterStatsComponent* Stats);

	// 由 Controller 在选中角色变化时调用：把角色数据资产（头像/名称）喂给状态条
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void BindCharacterData(UFFCharacterData* CharacterData);

	UFUNCTION(BlueprintPure, Category = "Menu")
	bool IsBound() const { return BoundEquipment != nullptr; }

	// 菜单控件（BP 里绑定在横向容器区域）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UFFHorizontalMenuWidget> HorizontalMenu;

	// 状态条控件（BP 里绑定，显示血量/体力）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UFFCharacterStatusWidget> StatusWidget;

private:
	UPROPERTY()
	TObjectPtr<UFFEquipmentComponent> BoundEquipment;
};