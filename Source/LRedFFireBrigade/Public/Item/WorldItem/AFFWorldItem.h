// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/Interaction/AFFInteractableActor.h"
#include "GameplayTagContainer.h"
#include "AFFWorldItem.generated.h"

class UAnimMontage;
class UStaticMeshComponent;
class UFFItemData;

// 世界物品基类：可交互基类 + "拾取/获得"统一流程。
// 自带 StaticMesh（BP 填网格资产即可，检测通道已配好）；交互时 GiveTo(User) → OnTaken()。
UCLASS(Abstract)
class LREDFFIREBRIGADE_API AFFWorldItem : public AFFInteractableActor
{
	GENERATED_BODY()

public:
	AFFWorldItem();

	// 该物品代表什么
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (Categories = "Item"))
	FGameplayTag ItemTag;

	// 显示名称（入装备后展示用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	// 物品静态定义（图标/名称/标签），BP 里填对应 DA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UFFItemData> ItemData;

	// 物品网格（视觉 + 可被交互检测）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	UStaticMeshComponent* Mesh;

	// 统一交互：播放拾取动作(可选) → GiveTo → OnTaken
	virtual void Interact(AActor* User) override;

	// 世界物品 = 纯拾取物，只能由 F 拾取，通用交互(E/右键)会跳过
	virtual bool IsPickup() const override { return true; }

	// 本物品的拾取动画：默认取实例字段 PickupMontage；装备基类覆写以提供"共享默认"
	virtual UAnimMontage* GetPickupMontage() const { return PickupMontage; }

	// 是否已被取走（已取走则不再参与交互/拾取判定）
	bool IsTaken() const { return bTaken; }

	// 把物品丢回地图（OnTaken 的反操作）：重新显示 + 开启碰撞，落点吸附到地面
	void DropAt(AActor* DropOwner, const FVector& Location);

protected:
	// 拾取动作（配了则播，会占用"忙"）
	UPROPERTY(EditAnywhere, Category = "Item")
	UAnimMontage* PickupMontage;

	// 把物品交给使用者（子类实现：入背包/装备等）
	virtual void GiveTo(AActor* User);

	// 取走后的收尾（默认隐藏 + 关碰撞）
	virtual void OnTaken();

	bool bTaken = false;
};
