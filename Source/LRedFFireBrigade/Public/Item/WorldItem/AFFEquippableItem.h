// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/WorldItem/AFFWorldItem.h"
#include "AFFEquippableItem.generated.h"

// 可装备物：交给使用者 = 放进其装备组件（背上表现后续另做）
UCLASS(Abstract)
class LREDFFIREBRIGADE_API AFFEquippableItem : public AFFWorldItem
{
	GENERATED_BODY()

public:
	AFFEquippableItem();

	// 拾取动画：实例(PickupMontage)未单独指定时，用装备共享默认（所有装备统一）
	virtual UAnimMontage* GetPickupMontage() const override;

protected:
	virtual void GiveTo(AActor* User) override;

private:
	// 所有装备的共享拾取动画（构造时从资源加载，个别装备可另在 PickupMontage 指定）
	UPROPERTY(EditDefaultsOnly, Category = "Item|Pickup")
	UAnimMontage* SharedPickupMontage;
};
