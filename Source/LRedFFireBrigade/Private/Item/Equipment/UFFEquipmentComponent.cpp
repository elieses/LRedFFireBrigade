// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Equipment/UFFEquipmentComponent.h"
#include "Item/Items/UFFItem.h"
#include "Item/WorldItem/AFFWorldItem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

FGameplayTag UFFEquipmentComponent::GetSlotTag(EFFEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EFFEquipmentSlot::Helmet:
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Helmet")));
	case EFFEquipmentSlot::Suit:
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Suit")));
	case EFFEquipmentSlot::Hand:
	default:
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Hand")));
	}
}

int32 UFFEquipmentComponent::GetHandCapacity() const
{
	return FMath::Max(0, FMath::Max(3, MaxSlots) - 2);
}

EFFEquipmentSlot UFFEquipmentComponent::GetSlotForTag(const FGameplayTag& EquipTag) const
{
	if (!EquipTag.IsValid())
	{
		return EFFEquipmentSlot::Hand;
	}

	if (EquipTag.MatchesTag(GetSlotTag(EFFEquipmentSlot::Helmet)))
	{
		return EFFEquipmentSlot::Helmet;
	}
	if (EquipTag.MatchesTag(GetSlotTag(EFFEquipmentSlot::Suit)))
	{
		return EFFEquipmentSlot::Suit;
	}
	return EFFEquipmentSlot::Hand;
}

UFFItem* UFFEquipmentComponent::EquipItem(UFFItem* NewItem)
{
	if (!NewItem)
	{
		return nullptr;
	}

	const EFFEquipmentSlot Slot = GetSlotForTag(NewItem->GetItemTag());

	UFFItem* Replaced = nullptr;
	switch (Slot)
	{
	case EFFEquipmentSlot::Helmet:
		Replaced = HelmetItem;
		HelmetItem = NewItem;
		break;

	case EFFEquipmentSlot::Suit:
		Replaced = SuitItem;
		SuitItem = NewItem;
		break;

	case EFFEquipmentSlot::Hand:
	default:
		if (HandItems.Num() < GetHandCapacity())
		{
			HandItems.Add(NewItem);   // 有空槽 → 装填
		}
		else
		{
			Replaced = HandItems[0];   // 全满 → 替换第一把，待调用方丢弃
			HandItems[0] = NewItem;
		}
		break;
	}

	// 手持槽变更后修正激活索引：新入手持物时默认激活第一件；越界则回退最近有效项
	if (Slot == EFFEquipmentSlot::Hand)
	{
		if (HandItems.Num() > 0 && ActiveHandIndex == EmptyHandIndex)
		{
			ActiveHandIndex = 0;
		}
		ActiveHandIndex = FMath::Clamp(ActiveHandIndex, 0, FMath::Max(0, HandItems.Num() - 1));
		if (HandItems.Num() == 0)
		{
			ActiveHandIndex = EmptyHandIndex;
		}
	}

	OnEquipmentChanged.Broadcast();
	return Replaced;
}

UFFItem* UFFEquipmentComponent::UnequipSlot(EFFEquipmentSlot Slot)
{
	UFFItem* Removed = nullptr;
	switch (Slot)
	{
	case EFFEquipmentSlot::Helmet:
		Removed = HelmetItem;
		HelmetItem = nullptr;
		break;
	case EFFEquipmentSlot::Suit:
		Removed = SuitItem;
		SuitItem = nullptr;
		break;
	case EFFEquipmentSlot::Hand:
	default:
		if (HandItems.Num() > 0)
		{
			Removed = HandItems[0];
			HandItems.RemoveAt(0);
		}
		break;
	}

	// 手持槽卸下后修正激活索引
	if (Slot == EFFEquipmentSlot::Hand)
	{
		if (HandItems.Num() == 0)
		{
			ActiveHandIndex = EmptyHandIndex;
		}
		else
		{
			ActiveHandIndex = FMath::Clamp(ActiveHandIndex, 0, HandItems.Num() - 1);
		}
	}

	if (Removed)
	{
		OnEquipmentChanged.Broadcast();
	}
	return Removed;
}

UFFItem* UFFEquipmentComponent::GetItem(EFFEquipmentSlot Slot) const
{
	switch (Slot)
	{
	case EFFEquipmentSlot::Helmet:
		return HelmetItem;
	case EFFEquipmentSlot::Suit:
		return SuitItem;
	case EFFEquipmentSlot::Hand:
	default:
		return HandItems.Num() > 0 ? HandItems[0].Get() : nullptr;
	}
}

bool UFFEquipmentComponent::HasItem(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return false;
	}

	for (EFFEquipmentSlot Slot : { EFFEquipmentSlot::Helmet, EFFEquipmentSlot::Suit, EFFEquipmentSlot::Hand })
	{
		if (UFFItem* Item = GetItem(Slot))
		{
			if (Item->GetItemTag() == Tag)
			{
				return true;
			}
		}
	}
	return false;
}

TArray<UFFItem*> UFFEquipmentComponent::GetItems() const
{
	TArray<UFFItem*> Result;
	if (HelmetItem)
	{
		Result.Add(HelmetItem);
	}
	if (SuitItem)
	{
		Result.Add(SuitItem);
	}
	for (const TObjectPtr<UFFItem>& Item : HandItems)
	{
		if (Item)
		{
			Result.Add(Item);
		}
	}
	return Result;
}

void UFFEquipmentComponent::SelectHandSlot(int32 Index)
{
	if (Index < 0 || Index >= HandItems.Num())
	{
		return;
	}
	if (ActiveHandIndex == Index)
	{
		return;
	}
	ActiveHandIndex = Index;
	OnEquipmentChanged.Broadcast();
}

void UFFEquipmentComponent::SelectEmptyHands()
{
	if (ActiveHandIndex == EmptyHandIndex)
	{
		return;
	}
	ActiveHandIndex = EmptyHandIndex;
	OnEquipmentChanged.Broadcast();
}

UFFItem* UFFEquipmentComponent::GetActiveHandItem() const
{
	if (ActiveHandIndex == EmptyHandIndex || !HandItems.IsValidIndex(ActiveHandIndex))
	{
		return nullptr;
	}
	return HandItems[ActiveHandIndex].Get();
}

TArray<FFFEquipmentEntry> UFFEquipmentComponent::GetSlotEntries() const
{
	TArray<FFFEquipmentEntry> Entries;

	auto AppendEntry = [this](TArray<FFFEquipmentEntry>& Out, EFFEquipmentSlot Slot, UFFItem* Item, bool bSetActive)
	{
		FFFEquipmentEntry E;
		E.Slot = Slot;
		E.Item = Item;
		E.bIsActive = bSetActive;
		if (Item)
		{
			E.Icon = Item->GetIcon();
			E.DisplayName = Item->GetDisplayName();
		}
		Out.Add(E);
	};

	AppendEntry(Entries, EFFEquipmentSlot::Helmet, HelmetItem, false);
	AppendEntry(Entries, EFFEquipmentSlot::Suit, SuitItem, false);

	for (int32 i = 0; i < GetHandCapacity(); ++i)
	{
		AppendEntry(Entries, EFFEquipmentSlot::Hand, HandItems.IsValidIndex(i) ? HandItems[i].Get() : nullptr,
			ActiveHandIndex == i);
	}

	// 末尾追加"空手"格（切换空手用）；空手状态下该格高亮
	if (bShowEmptyHandSlot)
	{
		FFFEquipmentEntry Empty;
		Empty.Slot = EFFEquipmentSlot::Hand;
		Empty.Item = nullptr;
		Empty.bIsActive = (ActiveHandIndex == EmptyHandIndex);
		Empty.DisplayName = FText::FromString(TEXT("空手"));
		Entries.Add(Empty);
	}

	return Entries;
}

void UFFEquipmentComponent::DropItem(AActor* Owner, UFFItem* OldItem)
{
	if (!OldItem)
	{
		return;
	}

	AFFWorldItem* OldWorld = OldItem->GetSourceWorldItem();
	if (OldWorld && Owner)
	{
		// 直接丢在角色脚下：取脚底（胶囊中心扣半径），防止落在头顶
		FVector OwnerFeet = Owner->GetActorLocation();
		if (const ACharacter* Char = Cast<ACharacter>(Owner))
		{
			OwnerFeet.Z -= Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
		OldWorld->DropAt(Owner, OwnerFeet);
	}

	OldItem->MarkAsGarbage();
}