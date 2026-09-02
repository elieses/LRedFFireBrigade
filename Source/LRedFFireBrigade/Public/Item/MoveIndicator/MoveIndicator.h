// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoveIndicator.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * 右键移动命令指示器：生成后停留在原地，直到目标角色到达目的地，
 * 随后缩小到 0 并销毁。下发新移动指令时旧指示器由控制器直接销毁。
 */
UCLASS()
class LREDFFIREBRIGADE_API AMoveIndicator : public AActor
{
	GENERATED_BODY()

public:
	AMoveIndicator();
	virtual void Tick(float DeltaTime) override;

	// 生成后调用：绑定要等待的目标角色和目的地
	void Initialize(AActor* InTarget, const FVector& InDestination);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Indicator")
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Indicator")
	UStaticMeshComponent* IndicatorMesh;

	// 到达判定距离（cm）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indicator")
	float ArriveThreshold = 80.f;
	// 兜底超时（秒）：导航失败时强制消失
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indicator")
	float MaxLifetime = 8.f;
	// 缩小淡出时长（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Indicator")
	float FadeDuration = 0.3f;

private:
	// 等待阶段：检测目标是否到达 / 移动失败 / 兜底超时
	void UpdateWaiting(float DeltaTime);
	// 淡出阶段：缩小到 0 后销毁
	void UpdateFade(float DeltaTime);

	TWeakObjectPtr<AActor> TargetActor;
	FVector Destination = FVector::ZeroVector;
	float Elapsed = 0.f;
	float FadeElapsed = 0.f;
	bool bArrived = false;
};
