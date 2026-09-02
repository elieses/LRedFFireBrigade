// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/MoveIndicator/MoveIndicator.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AIController.h"


AMoveIndicator::AMoveIndicator()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
	IndicatorMesh->SetupAttachment(Root);
	IndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AMoveIndicator::Initialize(AActor* InTarget, const FVector& InDestination)
{
	TargetActor = InTarget;
	Destination = InDestination;
}


void AMoveIndicator::BeginPlay()
{
	Super::BeginPlay();
	Elapsed = 0.f;
	FadeElapsed = 0.f;
	bArrived = false;
}


void AMoveIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 尚未到达：等待目标角色靠近目的地（或移动失败 / 兜底超时）
	if (!bArrived)
	{
		UpdateWaiting(DeltaTime);   // 可能把 bArrived 置 true
		if (!bArrived) return;      // 继续等待，保持完整大小
	}

	// 到达后：缩小淡出，结束后销毁
	UpdateFade(DeltaTime);
}


void AMoveIndicator::UpdateWaiting(float DeltaTime)
{
	Elapsed += DeltaTime;

	bool bReached = false;
	bool bFailed = false;
	if (TargetActor.IsValid())
	{
		const float Distance2D = FVector::Dist2D(TargetActor->GetActorLocation(), Destination);
		bReached = Distance2D <= ArriveThreshold;

		// 移动失败检测：角色已停下但未到目标 → 判定到不了
		if (APawn* Pawn = Cast<APawn>(TargetActor.Get()))
		{
			if (AAIController* AIController = Cast<AAIController>(Pawn->GetController()))
			{
				if (!AIController->IsFollowingAPath() && !bReached)
				{
					bFailed = true;
				}
			}
		}
	}

	if (bReached || bFailed || Elapsed >= MaxLifetime)
	{
		bArrived = true;
		FadeElapsed = 0.f;
	}
}


void AMoveIndicator::UpdateFade(float DeltaTime)
{
	FadeElapsed += DeltaTime;
	if (FadeElapsed >= FadeDuration)
	{
		Destroy();
		return;
	}

	const float T = FMath::Clamp(FadeElapsed / FadeDuration, 0.f, 1.f);
	const float Scale = 1.f - T;
	IndicatorMesh->SetRelativeScale3D(FVector(Scale, Scale, Scale));
}
