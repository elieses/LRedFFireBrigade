// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IFogRevealable.generated.h"

UINTERFACE()
class UFogRevealable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可被消防员"显示球体"笼罩的物体接口。
 * 采用引用计数: 每个罩住本物体的消防员都 AddRevealSource 一次,
 * 计数 >0 显示, ==0 隐藏。这样多个角色重叠时不会因一个离开而误隐藏。
 */
class LREDFFIREBRIGADE_API IFogRevealable
{
	GENERATED_BODY()

public:
	// 增加一个可见源 (被一个消防员的球体罩住)
	virtual void AddRevealSource() = 0;
	// 减少一个可见源 (离开该消防员的球体)
	virtual void RemoveRevealSource() = 0;
};
