// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractor.generated.h"

class AActor;
class UAnimMontage;

UINTERFACE()
class UInteractor : public UInterface
{
	GENERATED_BODY()
};

// 能主动与可交互物体交互的单位
class LREDFFIREBRIGADE_API IInteractor
{
	GENERATED_BODY()

public:
	// E 触发：交互范围内最近的可交互物
	virtual void TryInteract() = 0;

	// 右键指定物体：走位到目标后执行交互
	virtual void CommandInteract(AActor* Target) = 0;

	// 播放一次交互动作并"忙"到动画结束（无蒙太奇则忽略）
	virtual void PlayActionMontage(UAnimMontage* Montage) = 0;

	// F 专用拾取：只拾取交互半径内最近的世界物品（AFFWorldItem），绝不触发开门/开关
	virtual void TryPickup() = 0;
};
