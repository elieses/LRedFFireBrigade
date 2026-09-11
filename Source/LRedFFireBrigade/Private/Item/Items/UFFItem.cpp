// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Items/UFFItem.h"
#include "Item/Data/UFFItemData.h"
#include "Item/WorldItem/AFFWorldItem.h"
#include "Engine/Texture2D.h"

void UFFItem::SetSourceWorldItem(AFFWorldItem* Item)
{
	SourceWorldItem = Item;
}

AFFWorldItem* UFFItem::GetSourceWorldItem() const
{
	return SourceWorldItem.Get();
}

FText UFFItem::GetDisplayName() const
{
	if (ItemDataRef && !ItemDataRef->DisplayName.IsEmpty())
	{
		return ItemDataRef->DisplayName;
	}
	return FText::GetEmpty();
}

UTexture2D* UFFItem::GetIcon() const
{
	return ItemDataRef ? ItemDataRef->Icon : nullptr;
}

FGameplayTag UFFItem::GetItemTag() const
{
	return ItemDataRef ? ItemDataRef->ItemTag : ItemTag;
}