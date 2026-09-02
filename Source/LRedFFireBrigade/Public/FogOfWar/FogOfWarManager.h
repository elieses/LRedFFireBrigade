// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogOfWarManager.generated.h"

class UTextureRenderTarget2D;
class UTexture2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class APostProcessVolume;

/**
 * 战争迷雾管理器
 *
 * 原理:
 *  - 对每个可操作单位发一圈水平射线(LineTrace, 打建筑/墙体), 得到各方向可见距离。
 *  - 用 "印章材质" 把可见多边形加性盖进三张渲染目标:
 *      LiveRTs[2]  (实时可见层, 逐帧 ping-pong 衰减 -> 实现 "离开后变暗残影")
 *      ExploredRT  (累积探索层, 永不衰减 -> 实现 "永久探索")
 *  - 后处理材质按三态混合: 当前可见=清晰 / 已探索残影=半雾 / 未探索=浓雾。
 */
UCLASS()
class LREDFFIREBRIGADE_API AFogOfWarManager : public AActor
{
	GENERATED_BODY()

public:
	AFogOfWarManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// ---- 材质引用 (在编辑器 / BP 里指定) ----
	// 盖章用: 输出一个软边三角形(参数 P0/P1/P2, SoftEdge), Blend=Additive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Materials")
	UMaterialInterface* RevealStampMaterial;
	// 衰减用: 采样雾图 x FadeAmount, Blend=Opaque (用于实时可见层每帧衰减)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Materials")
	UMaterialInterface* FadeMaterial;
	// 最终呈现: 后处理材质, 采样两张雾图做三态混合
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Materials")
	UMaterialInterface* FogPostProcessMaterial;

	// ---- 设置 ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	int32 FogResolution = 1024;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float WorldMinX = -5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float WorldMaxX = 5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float WorldMinY = -5000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float WorldMaxY = 5000.f;
	// 可见半径 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float VisionRadius = 1500.f;
	// 一圈采样几条射线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	int32 RayCount = 48;
	// 更新间隔 (s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float UpdateInterval = 0.1f;
	// 残影淡出速度 (越大淡出越快)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float FadeSpeed = 1.5f;
	// 射线起始高度 (相对地面, cm), 避免打到地面
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float TraceHeight = 100.f;
	// 印章软边宽度 (0~1, 越小边缘越锐利)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	float StampSoftEdge = 0.05f;
	// 径向距离图宽度 (角度采样精度, >= 实际射线数更平滑)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogOfWar|Settings")
	int32 RadialDistanceResolution = 64;

	// ---- 运行时 (调试用) ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogOfWar|Debug")
	UTextureRenderTarget2D* LiveFogRT;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogOfWar|Debug")
	UTextureRenderTarget2D* ExploredRT;
	// 当前单位各角度的可见距离图 (UploadRadialDistances 写入, 印章材质按角度采样)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogOfWar|Debug")
	UTexture2D* RadialDistanceTexture;

protected:
	// 世界坐标 -> 雾图UV
	FVector2D WorldToFogUV(const FVector& WorldPos) const;
	// 对单个单位做一圈射线采样, 输出各方向可见距离 (世界单位 cm)
	void SampleUnitVision(const AActor* Unit, TArray<float>& OutDistances);
	// 把 RayCount 条距离插值填充并上传到 RadialDistanceTexture (UV单位 0~1)
	void UploadRadialDistances(const TArray<float>& Distances);
	// 每单位一次全屏绘制: 把 (Center, Distances) 围成的可见多边形盖章进指定RT (加性)
	void StampRevealGPU(const FVector& Center, const TArray<float>& Distances, UTextureRenderTarget2D* Target);
	// 定时器回调: 收集所有可操作单位并更新两张雾图
	void UpdateFog();
	// 每帧把实时可见层 ping-pong 衰减
	void FadeVisibility(float DeltaSeconds);
	// 创建后处理体积并绑定雾材质
	void SetupPostProcess();

	// 两张实时可见层, ping-pong 用
	UPROPERTY(VisibleAnywhere, Category="FogOfWar|Debug")
	UTextureRenderTarget2D* LiveRTs[2];
	int32 CurrentLiveIndex = 0;

	FTimerHandle UpdateTimerHandle;
	UPROPERTY()
	UMaterialInstanceDynamic* RevealStampMID;
	UPROPERTY()
	UMaterialInstanceDynamic* FadeMID;
	UPROPERTY()
	UMaterialInstanceDynamic* FogPostProcessMID;
	UPROPERTY()
	APostProcessVolume* PostProcessVolume;
};
