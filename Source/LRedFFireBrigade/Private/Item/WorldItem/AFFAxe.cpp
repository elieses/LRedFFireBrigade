// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WorldItem/AFFAxe.h"

AFFAxe::AFFAxe()
{
	ItemTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Hand.Axe")));
}
