// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "UFFItem.generated.h"

class AFFWorldItem;
class UFFItem;
class UFFItemData;
class UTexture2D;

// 装备槽位：每个角色拥有若干槽（头盔 / 防火服 / 手持×N）。多出的槽全部归手持类。
UENUM(BlueprintType)
enum class EFFEquipmentSlot : uint8
{
	Helmet,
	Suit,
	Hand
};

// 背包单个格子展示用的条目（空槽 item 为 null）
USTRUCT(BlueprintType)
struct FFFEquipmentEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	EFFEquipmentSlot Slot = EFFEquipmentSlot::Hand;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UFFItem> Item;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	FText DisplayName;

	// 该格是否为当前激活（手持中）状态
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	bool bIsActive = false;
};

// 物品实例（装备组件槽位里保存的元素；持有所属静态定义 + 来源世界物体引用）
UCLASS(Blueprintable)
class LREDFFIREBRIGADE_API UFFItem : public UObject
{
	GENERATED_BODY()

public:
	// 物品静态定义（图标/名称/标签），拾取时从世界物带入
	UPROPERTY()
	TObjectPtr<UFFItemData> ItemDataRef;

	// （兼容保留）若没有 ItemDataRef 时的兜底
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGameplayTag ItemTag;

	// 该物品取自哪个世界物体（丢回地图/替换时复用其网格）
	void SetSourceWorldItem(AFFWorldItem* Item);
	AFFWorldItem* GetSourceWorldItem() const;

	// UI 取用：优先静态定义，缺失时用兜底字段
	FText GetDisplayName() const;
	UTexture2D* GetIcon() const;
	FGameplayTag GetItemTag() const;

private:
	UPROPERTY()
	TWeakObjectPtr<AFFWorldItem> SourceWorldItem;
};