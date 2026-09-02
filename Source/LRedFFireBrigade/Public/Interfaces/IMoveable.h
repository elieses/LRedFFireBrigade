// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IMoveable.generated.h"

UINTERFACE()
class UMoveable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可被右键命令移动到指定位置的单位
 */
class LREDFFIREBRIGADE_API IMoveable
{
	GENERATED_BODY()

public:
	virtual void MoveToLocation(const FVector& Destination) = 0;
};
