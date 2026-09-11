// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFFItemUser.generated.h"

class UFFEquipmentComponent;

UINTERFACE()
class UFFItemUser : public UInterface
{
	GENERATED_BODY()
};

// 能携带物品的角色（有 UFFEquipmentComponent）
class LREDFFIREBRIGADE_API IFFItemUser
{
	GENERATED_BODY()

public:
	virtual UFFEquipmentComponent* GetEquipment() const = 0;
};
