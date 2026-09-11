// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Characters/Firefighters/FirefighterBase.h"
#include "UFFFirefighterAnimInstance.generated.h"

// 消防员动画蓝图基类：每帧缓存角色状态，供 ABP 直接读取
UCLASS()
class LREDFFIREBRIGADE_API UFFFirefighterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// —— 内联读取（供动画蓝图使用）——
	UFUNCTION(BlueprintPure, Category = "FF|State")
	ECharacterState GetCharacterState() const { return CachedCharacterState; }

	UFUNCTION(BlueprintPure, Category = "FF|State")
	EFFMovementState GetMovementState() const { return CachedMovementState; }

	UFUNCTION(BlueprintPure, Category = "FF|State")
	bool IsFirefighterBusy() const { return bCachedBusy; }

	UFUNCTION(BlueprintPure, Category = "FF|State")
	float GetGroundSpeed() const { return CachedGroundSpeed; }

	UFUNCTION(BlueprintPure, Category = "FF|State")
	bool IsInAir() const { return bCachedInAir; }

protected:
	// 每帧由 NativeUpdateAnimation 更新，C++/蓝图只读
	UPROPERTY(BlueprintReadOnly, Category = "FF|State")
	ECharacterState CachedCharacterState = ECharacterState::ECS_UnEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "FF|State")
	EFFMovementState CachedMovementState = EFFMovementState::Walk;

	UPROPERTY(BlueprintReadOnly, Category = "FF|State")
	bool bCachedBusy = false;

	UPROPERTY(BlueprintReadOnly, Category = "FF|State")
	float CachedGroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "FF|State")
	bool bCachedInAir = false;
};