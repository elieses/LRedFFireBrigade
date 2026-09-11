// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UFFCharacterStatsComponent.generated.h"

// 状态数据变化委托（View 订阅即时刷新）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFFOnStatsChanged);

// 角色状态组件：体力 + 血量。值改变时广播 OnStatsChanged。
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LREDFFIREBRIGADE_API UFFCharacterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 数据变化事件（Apply/Set 且值有变化时广播）
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FFFOnStatsChanged OnStatsChanged;

	// 血量/体力上限（编辑器可调；每 BP 可不同）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.f;

	// —— 游戏逻辑入口（以此改值，自动 clamp + 广播）——
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyHealthDelta(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyStaminaDelta(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetHealth(float Value);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStamina(float Value);

	// 设置上限（角色从 DataAsset 套用时用）；重夹取当前值并广播
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetMaxHealth(float Value);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetMaxStamina(float Value);

	// —— UI/逻辑取用 ——
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStaminaPercent() const { return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Health = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Stamina = 100.f;
};