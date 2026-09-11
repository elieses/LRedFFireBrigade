// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WorldItem/AFFHelmet.h"

AFFHelmet::AFFHelmet()
{
	ItemTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Helmet.Standard")));
}