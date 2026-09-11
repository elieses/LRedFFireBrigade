// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UFFItemData.generated.h"

class UTexture2D;
class UStaticMesh;

// 物品静态定义（每类物品一个 DataAsset）
UCLASS(BlueprintType)
class LREDFFIREBRIGADE_API UFFItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 物品标签（决定槽位类别与逻辑身份）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (Categories = "Equipment"))
	FGameplayTag ItemTag;

	// 显示名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	// 描述（可选，tooltip 用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText Description;

	// 背包图标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UTexture2D> Icon;

	// 持握方式：单手 / 双手（角色状态与动画用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (Categories = "Equipment.Grip"))
	FGameplayTag GripTag;

	// 手持表现网格（切换到该装备时挂到角色手部插槽）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UStaticMesh> EquipmentMesh;

	// 是否能劈砍破门（斧头勾上；未来其它工具也可用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanBreakDoor = false;

	// 劈砍破门消耗的体力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float BreakDoorStaminaCost = 40.f;
};