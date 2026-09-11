// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UFFCharacterStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UFFCharacterStatsComponent;
class UFFCharacterData;

// 角色状态 UI（MVC 的 View）：两条进度条（血量/体力）+ 数字。
// 订阅 Stats 组件 OnStatsChanged → 值一变化即时刷新。
UCLASS()
class LREDFFIREBRIGADE_API UFFCharacterStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 由背包/Controller 调用：绑定状态组件并立即刷新一次
	UFUNCTION(BlueprintCallable, Category = "Status")
	void BindStats(UFFCharacterStatsComponent* Stats);

	// 由背包/Controller 调用：绑定角色数据资产（头像/名称）
	UFUNCTION(BlueprintCallable, Category = "Status")
	void BindCharacterData(UFFCharacterData* CharacterData);

	// 头像（BP 里命名 AvatarImage）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> AvatarImage;

	// 角色名（BP 里命名 CharacterNameText）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterNameText;

	// 血量进度条（BP 里命名 HealthBar）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// 体力进度条（BP 里命名 StaminaBar）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	// 血量数字（BP 里命名 HealthText，显示如 "75/100"）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	// 血量百分比（BP 里命名 HealthPercentText，显示如 "75%"）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthPercentText;

	// 体力数字（BP 里命名 StaminaText，显示如 "75/100"）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> StaminaText;

	// 体力百分比（BP 里命名 StaminaPercentText，显示如 "75%"）
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> StaminaPercentText;

protected:
	// 状态变化回调：刷新两条进度条与数字
	UFUNCTION()
	void OnStatsChanged();

	// 用当前数值刷新条与文字
	void RefreshBars();

	TWeakObjectPtr<UFFCharacterStatsComponent> BoundStats;

	bool bBound = false;

	TWeakObjectPtr<UFFCharacterData> BoundCharacterData;
};