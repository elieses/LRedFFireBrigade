// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IFogRevealable.h"
#include "HideableBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/**
 * 受战争迷雾"显示范围"控制的物体基类。
 * 默认出生时隐藏; 被消防员的显示球体罩住(计数>0)时显示, 全部离开后重新隐藏。
 */
UCLASS()
class LREDFFIREBRIGADE_API AHideableBase : public AActor, public IFogRevealable
{
	GENERATED_BODY()

public:
	AHideableBase();
	virtual void BeginPlay() override;

	// IFogRevealable
	virtual void AddRevealSource() override;
	virtual void RemoveRevealSource() override;

protected:
	// 可见源计数: >0 显示, ==0 隐藏
	int32 RevealSourceCount = 0;
	void UpdateVisibility();

public:
	// 静态网格体 (根组件, 用于显示物体外观)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogReveal")
	UStaticMeshComponent* Mesh;
	// 用于被消防员球体检测到的碰撞体 (仅Query, 只响应 FireFighter 通道)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogReveal")
	USphereComponent* FogCollider;
	// 出生时是否隐藏 (默认 true: 需玩家靠近才显示)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogReveal")
	bool bStartHidden = true;
};
