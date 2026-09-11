// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UFFCharacterData.generated.h"

class UTexture2D;

// 角色静态定义（每个角色一个 DataAsset：头像/名称/属性上限）
UCLASS(BlueprintType)
class LREDFFIREBRIGADE_API UFFCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 角色名（状态条显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText DisplayName;

	// 头像（状态条显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TObjectPtr<UTexture2D> ProfilePicture;

	// 血量上限（从数据资产获取，角色初始化血量）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float MaxHealth = 100.f;

	// 体力上限
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float MaxStamina = 100.f;

// 移动速度（Z 切换走/跑；蹲下用 CrouchSpeed）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Move")
	float WalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Move")
	float RunSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Move")
	float CrouchSpeed = 80.f;

	// 体力：跑步每秒消耗 / 走路每秒回复 / 蹲下每秒回复 / 静止不动每秒回复
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float RunStaminaDrain = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float WalkStaminaRegen = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float CrouchStaminaRegen = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float IdleStaminaRegen = 10.f;

	// 力竭后需回复到此值才能再次奔跑
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stamina")
	float RunStaminaResume = 40.f;
};