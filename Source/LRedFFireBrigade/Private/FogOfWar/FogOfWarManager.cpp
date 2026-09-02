// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWar/FogOfWarManager.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "CollisionQueryParams.h"
#include "Characters/Firefighters/FirefighterBase.h"

// VisibilityTrace 通道: 在 DefaultEngine.ini 里定义为 ECC_GameTraceChannel3
namespace
{
	const ECollisionChannel VisibilityTraceChannel = ECC_GameTraceChannel3;
}

AFogOfWarManager::AFogOfWarManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFogOfWarManager::BeginPlay()
{
	Super::BeginPlay();

	if (RevealStampMaterial)
	{
		RevealStampMID = UMaterialInstanceDynamic::Create(RevealStampMaterial, this);
	}
	if (FadeMaterial)
	{
		FadeMID = UMaterialInstanceDynamic::Create(FadeMaterial, this);
	}
	if (FogPostProcessMaterial)
	{
		FogPostProcessMID = UMaterialInstanceDynamic::Create(FogPostProcessMaterial, this);
	}

	const FLinearColor Black(0.f, 0.f, 0.f, 1.f);
	for (int32 i = 0; i < 2; ++i)
	{
		LiveRTs[i] = UKismetRenderingLibrary::CreateRenderTarget2D(this, FogResolution, FogResolution, RTF_RGBA8, Black);
	}
	ExploredRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, FogResolution, FogResolution, RTF_RGBA8, Black);
	LiveFogRT = LiveRTs[CurrentLiveIndex];

	// 创建径向距离图 (宽=分辨率, 高=1): 各列存一个角度bin的可见距离
	if (RadialDistanceResolution > 0)
	{
		RadialDistanceTexture = UTexture2D::CreateTransient(RadialDistanceResolution, 1, PF_B8G8R8A8);
		if (RadialDistanceTexture)
		{
			RadialDistanceTexture->SRGB = false;
			RadialDistanceTexture->Filter = TF_Bilinear;
			RadialDistanceTexture->AddressX = TA_Wrap;  // 角度环绕
			RadialDistanceTexture->AddressY = TA_Clamp;
			RadialDistanceTexture->CompressionSettings = TC_VectorDisplacementmap;
			RadialDistanceTexture->UpdateResource();
		}
	}

	SetupPostProcess();

	GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &AFogOfWarManager::UpdateFog, UpdateInterval, true);
}

void AFogOfWarManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FadeVisibility(DeltaSeconds);
}

FVector2D AFogOfWarManager::WorldToFogUV(const FVector& WorldPos) const
{
	const float SizeX = WorldMaxX - WorldMinX;
	const float SizeY = WorldMaxY - WorldMinY;
	const float U = (SizeX > KINDA_SMALL_NUMBER) ? (WorldPos.X - WorldMinX) / SizeX : 0.f;
	const float V = (SizeY > KINDA_SMALL_NUMBER) ? (WorldPos.Y - WorldMinY) / SizeY : 0.f;
	return FVector2D(U, V);
}

void AFogOfWarManager::SampleUnitVision(const AActor* Unit, TArray<float>& OutDistances)
{
	OutDistances.SetNum(RayCount);
	UWorld* World = GetWorld();
	if (!World || !Unit)
	{
		for (int32 i = 0; i < RayCount; ++i)
		{
			OutDistances[i] = VisionRadius;
		}
		return;
	}

	FVector Origin = Unit->GetActorLocation();
	const FVector GroundCenter(Origin.X, Origin.Y, Origin.Z);
	Origin.Z += TraceHeight;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FogOfWarTrace), false);
	Params.AddIgnoredActor(Unit);

	for (int32 i = 0; i < RayCount; ++i)
	{
		const float Angle = (2.f * PI * i) / RayCount;
		const FVector Dir(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		const FVector End = Origin + Dir * VisionRadius;

		float Dist = VisionRadius;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Origin, End, VisibilityTraceChannel, Params) && Hit.bBlockingHit)
		{
			Dist = FVector::Dist2D(Hit.ImpactPoint, GroundCenter);
			Dist = FMath::Clamp(Dist, 0.f, VisionRadius);
		}
		OutDistances[i] = Dist;
	}
}

void AFogOfWarManager::UploadRadialDistances(const TArray<float>& Distances)
{
	if (!RadialDistanceTexture || Distances.Num() == 0)
	{
		return;
	}

	const float WorldSizeX = WorldMaxX - WorldMinX;
	const float WorldSizeY = WorldMaxY - WorldMinY;
	const float WorldSize = (WorldSizeX + WorldSizeY) * 0.5f; // 正方形地图
	const float MaxWorldDist = VisionRadius;

	const int32 SrcN = Distances.Num();
	const int32 DstN = RadialDistanceResolution;

	TArray<uint8> Data;
	Data.SetNum(DstN * 4);

	for (int32 Dst = 0; Dst < DstN; ++Dst)
	{
		// 目标角度: 匹配材质 angleNorm=(atan2+PI)/(2PI) 的采样约定。
		// 材质里 U=(atan2(delta)+PI)/(2PI) => 对应世界方向 angle=2*PI*U-PI。
		// 因此这里按 angle=2*PI*(D/DstN)-PI 填, 再折回 [0, 2PI) 对齐射线索引。
		float Angle = (2.f * PI * Dst) / DstN - PI;   // [-PI, PI)
		if (Angle < 0.f)
		{
			Angle += 2.f * PI;                        // 折回 [0, 2PI)
		}
		// 找到该角度在源射线中的浮点索引
		const float SrcIdx = (Angle / (2.f * PI)) * SrcN;
		const int32 Idx0 = FMath::Clamp(FMath::FloorToInt(SrcIdx), 0, SrcN - 1);
		const int32 Idx1 = (Idx0 + 1) % SrcN;
		const float Frac = SrcIdx - FMath::FloorToInt(SrcIdx);

		float Dist = FMath::Lerp(Distances[Idx0], Distances[Idx1], Frac);
		Dist = FMath::Clamp(Dist, 0.f, MaxWorldDist);

		// 归一化到 UV 单位 (0~1): 世界距离 / 地图边长
		float DistUV = (WorldSize > KINDA_SMALL_NUMBER) ? (Dist / WorldSize) : 0.f;
		DistUV = FMath::Clamp(DistUV, 0.f, 1.f);

		const uint8 V = (uint8)(FMath::RoundToInt(DistUV * 255.f));
		Data[Dst * 4 + 0] = V;
		Data[Dst * 4 + 1] = V;
		Data[Dst * 4 + 2] = V;
		Data[Dst * 4 + 3] = 255;
	}

	// 上传到纹理 (BulkData + UpdateResource)
	FTexture2DMipMap& Mip = RadialDistanceTexture->GetPlatformData()->Mips[0];
	void* DataPtr = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(DataPtr, Data.GetData(), Data.Num());
	Mip.BulkData.Unlock();
	RadialDistanceTexture->UpdateResource();
}

void AFogOfWarManager::StampRevealGPU(const FVector& Center, const TArray<float>& Distances, UTextureRenderTarget2D* Target)
{
	if (!RevealStampMID || !Target || !RadialDistanceTexture)
	{
		return;
	}

	UploadRadialDistances(Distances);

	const float WorldSizeX = WorldMaxX - WorldMinX;
	const float WorldSizeY = WorldMaxY - WorldMinY;
	const float WorldSize = (WorldSizeX + WorldSizeY) * 0.5f;

	const FVector2D CenterUV = WorldToFogUV(Center);
	const float RadiusUV = (WorldSize > KINDA_SMALL_NUMBER) ? (VisionRadius / WorldSize) : 0.f;

	RevealStampMID->SetVectorParameterValue("CenterUV", FLinearColor(CenterUV.X, CenterUV.Y, 0.f, 0.f));
	RevealStampMID->SetScalarParameterValue("RadiusUV", RadiusUV);
	RevealStampMID->SetScalarParameterValue("SoftEdge", StampSoftEdge);
	RevealStampMID->SetTextureParameterValue("RadialDistanceTexture", RadialDistanceTexture);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, Target, RevealStampMID);
}

void AFogOfWarManager::UpdateFog()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AFirefighterBase> It(World); It; ++It)
	{
		AFirefighterBase* Unit = *It;
		if (!IsValid(Unit))
		{
			continue;
		}

		TArray<float> Distances;
		SampleUnitVision(Unit, Distances);
		const FVector Center = Unit->GetActorLocation();
		StampRevealGPU(Center, Distances, LiveRTs[CurrentLiveIndex]);
		StampRevealGPU(Center, Distances, ExploredRT);
	}
}

void AFogOfWarManager::FadeVisibility(float DeltaSeconds)
{
	if (!FadeMID || !FogPostProcessMID)
	{
		return;
	}

	const int32 FadeDest = 1 - CurrentLiveIndex;
	const float FadeFactor = FMath::Clamp(1.f - FadeSpeed * DeltaSeconds, 0.f, 1.f);

	// 读取 source, 写入 dest (避免读写同一张RT的冲突)
	FadeMID->SetTextureParameterValue("FogTexture", LiveRTs[CurrentLiveIndex]);
	FadeMID->SetScalarParameterValue("FadeAmount", FadeFactor);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, LiveRTs[FadeDest], FadeMID);

	CurrentLiveIndex = FadeDest;
	LiveFogRT = LiveRTs[CurrentLiveIndex];

	// 让后处理材质采样当前最新的实时可见层
	FogPostProcessMID->SetTextureParameterValue("VisibilityTexture", LiveRTs[CurrentLiveIndex]);
}

void AFogOfWarManager::SetupPostProcess()
{
	UWorld* World = GetWorld();
	if (!World || !FogPostProcessMID)
	{
		return;
	}

	PostProcessVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform(FVector::ZeroVector));
	if (!PostProcessVolume)
	{
		return;
	}
	PostProcessVolume->bUnbound = true;
	PostProcessVolume->Priority = 1000.f;
	PostProcessVolume->BlendWeight = 1.f;

	FogPostProcessMID->SetTextureParameterValue("VisibilityTexture", LiveRTs[CurrentLiveIndex]);
	FogPostProcessMID->SetTextureParameterValue("ExploredTexture", ExploredRT);

	// 世界范围 (材质里用世界XY反算UV)
	FogPostProcessMID->SetScalarParameterValue("WorldMinX", WorldMinX);
	FogPostProcessMID->SetScalarParameterValue("WorldMinY", WorldMinY);
	FogPostProcessMID->SetScalarParameterValue("WorldSizeX", WorldMaxX - WorldMinX);
	FogPostProcessMID->SetScalarParameterValue("WorldSizeY", WorldMaxY - WorldMinY);

	PostProcessVolume->AddOrUpdateBlendable(FogPostProcessMID, 1.f);
}
