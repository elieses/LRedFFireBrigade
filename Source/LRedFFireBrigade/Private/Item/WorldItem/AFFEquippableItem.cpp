// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WorldItem/AFFEquippableItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimMontage.h"
#include "Interfaces/IFFItemUser.h"
#include "Item/Equipment/UFFEquipmentComponent.h"
#include "Item/Items/UFFItem.h"

AFFEquippableItem::AFFEquippableItem()
{
	// 所有装备共用的默认拾取动画（在构造函数设为类默认，子类未单独指定则统一用它）
	static ConstructorHelpers::FObjectFinder<UAnimMontage> SharedMontage(
		TEXT("/Game/Assets/Animation/FireFighter/Man_PickUp_Table_Montage.Man_PickUp_Table_Montage"));
	if (SharedMontage.Succeeded())
	{
		SharedPickupMontage = SharedMontage.Object;
	}
}


UAnimMontage* AFFEquippableItem::GetPickupMontage() const
{
	// 实例已单独指定则优先，否则统一用共享默认
	if (PickupMontage)
	{
		return PickupMontage;
	}
	return SharedPickupMontage;
}

void AFFEquippableItem::GiveTo(AActor* User)
{
	if (IFFItemUser* ItemUser = Cast<IFFItemUser>(User))
	{
		if (UFFEquipmentComponent* Equipment = ItemUser->GetEquipment())
		{
			UFFItem* NewItem = NewObject<UFFItem>(Equipment);
			NewItem->ItemDataRef = ItemData;              // 静态定义（图标/名称/标签）
			NewItem->ItemTag = ItemTag;                   // 兜底（无 ItemData 时用）
			NewItem->SetSourceWorldItem(this);            // 记录来源：替换时用它丢回地图

			UFFItem* Replaced = Equipment->EquipItem(NewItem);
			if (Replaced)
			{
				Equipment->DropItem(User, Replaced);   // 旧装备被替换 → 丢在角色脚下
			}
		}
	}
}
