// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Item/Items/UFFItem.h"
#include "UFFEquipmentComponent.generated.h"

// 装备组件数据变化委托（View 订阅刷新）
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFFOnEquipmentChanged);

// 角色装备组件：头盔×1、防火服×1、手持×N（MaxSlots 控制总槽数，多出的全部归手持）。
// 槽位类别由物品 ItemTag 的前缀推断（Equipment.Helmet / Equipment.Suit / Equipment.Hand）。
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LREDFFIREBRIGADE_API UFFEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 数据变化事件（Equip/Unequip 成功后广播，View 据此刷新）
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FFFOnEquipmentChanged OnEquipmentChanged;

	// 装备一个物品到它所属的槽；槽被占 → 返回被替换的旧物品（由调用方决定如何丢弃）
	UFFItem* EquipItem(UFFItem* NewItem);

	// 卸下指定槽的物品并返回它（没有物品则返回 nullptr）
	UFFItem* UnequipSlot(EFFEquipmentSlot Slot);

	// 取指定槽的物品（Hand 取第一把手持）
	UFFItem* GetItem(EFFEquipmentSlot Slot) const;

	// 是否拥有指定标签的物品（遍历所有槽）
	bool HasItem(const FGameplayTag& Tag) const;

	// 由物品标签判断所属槽位（无匹配前缀默认手持槽）
	EFFEquipmentSlot GetSlotForTag(const FGameplayTag& EquipTag) const;

	// 所有有效项汇总（供逻辑遍历）
	TArray<UFFItem*> GetItems() const;

	// 激活指定的手持槽（Index 越界则 clamp）；成功即广播 OnEquipmentChanged
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SelectHandSlot(int32 Index);

	// 切换到空手（ActiveHandIndex = -1）；广播 OnEquipmentChanged
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SelectEmptyHands();

	// 当前激活的手持物品（空手/无物 → nullptr）
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UFFItem* GetActiveHandItem() const;

	// 当前是否处于空手（未选中任何手持物）状态
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsEmptyHandsActive() const { return ActiveHandIndex == EmptyHandIndex; }

	// 当前激活的手持槽索引（-1 = 空手）
	UFUNCTION(BlueprintPure, Category = "Equipment")
	int32 GetActiveHandIndex() const { return ActiveHandIndex; }

	// UI 直读：返回固定 MaxSlots 条（空槽 = 空壳条目），顺序：头盔、防火服、手持×N；
	// 若 bShowEmptyHandSlot，末尾追加"空手格"
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<FFFEquipmentEntry> GetSlotEntries() const;

	// 槽位对应的类别父标签
	static FGameplayTag GetSlotTag(EFFEquipmentSlot Slot);

	// 替换旧物时：把旧物品丢回地图（位置 = 拥有者脚下），并销毁旧物品对象
	void DropItem(AActor* Owner, UFFItem* OldItem);

	// 总槽数（最少 3；超过部分自动成为手持槽）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	int32 MaxSlots = 3;

	// 是否在背包菜单末尾显示"空手"格（用于切换空手状态）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	bool bShowEmptyHandSlot = true;

protected:
	// 手持槽数量 = MaxSlots - 2（头盔/防火服各占 1）
	int32 GetHandCapacity() const;

	// 空手 = 未激活任何手持物
	static constexpr int32 EmptyHandIndex = -1;

	// 当前激活的手持槽索引（-1 = 空手）
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 ActiveHandIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	UFFItem* HelmetItem;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	UFFItem* SuitItem;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<TObjectPtr<UFFItem>> HandItems;
};