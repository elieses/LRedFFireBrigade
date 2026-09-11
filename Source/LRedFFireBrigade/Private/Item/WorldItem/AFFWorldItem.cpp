// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WorldItem/AFFWorldItem.h"
#include "Animation/AnimMontage.h"
#include "CollisionQueryParams.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInteractor.h"

AFFWorldItem::AFFWorldItem()
{
	// 物品网格作为根；只做检测（能被右键/E 的 CanInteractor 通道命中），不挡人
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel6 /*InteractorObject*/);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel5 /*CanInteractor*/, ECR_Overlap);
	// 物品仅作拾取静态摆放，永不进入物理模拟（避免 BP 勾了 Simulate Physics 导致坠落穿地）
	//Mesh->SetSimulatePhysics(false);
	//Mesh->SetEnableGravity(false);
}


void AFFWorldItem::Interact(AActor* User)
{
	if (bTaken)
	{
		return;
	}

	// 可选：拾取动作（经虚函数获取，装备基类可提供共享默认）
	UAnimMontage* MontageToPlay = GetPickupMontage();
	if (IInteractor* Unit = Cast<IInteractor>(User))
	{
		Unit->PlayActionMontage(MontageToPlay);
	}
	else if (ACharacter* Char = Cast<ACharacter>(User))
	{
		if (MontageToPlay)
		{
			Char->PlayAnimMontage(MontageToPlay);
		}
	}

	GiveTo(User);
	OnTaken();
}


void AFFWorldItem::GiveTo(AActor* User)
{
	// 子类实现
}


void AFFWorldItem::OnTaken()
{
	if (bTaken)
	{
		return;
	}
	bTaken = true;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}


void AFFWorldItem::DropAt(AActor* DropOwner, const FVector& Location)
{
	// 从挂载(若有)脱离，回到世界
	if (USceneComponent* Root = GetRootComponent())
	{
		Root->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
	if (DropOwner)
	{
		SetOwner(DropOwner);
	}

	// 反操作 OnTaken：复位标记、显示、开碰撞、强制退出物理（BP 勾了 Simulate 也会被关）
	bTaken = false;
	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel6 /*InteractorObject*/);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel5 /*CanInteractor*/, ECR_Overlap);
	Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Mesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);

	// 落点吸附地面：从目标上方往下打射线，命中处 +Z（约3cm）作为最终落地位置
	FVector DropPos = Location;
	if (UWorld* World = GetWorld())
	{
		const FVector TraceStart = Location + FVector(0, 0, 200.f);
		const FVector TraceEnd = Location - FVector(0, 0, 1000.f);
		FCollisionQueryParams QP;
		QP.AddIgnoredActor(this);
		QP.AddIgnoredActor(DropOwner);   // 绕过消防员身体：命中脚下的地面而非头顶

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QP))
		{
			DropPos = Hit.ImpactPoint + FVector(0, 0, 3.f);
		}
	}
	SetActorLocation(DropPos);
}
