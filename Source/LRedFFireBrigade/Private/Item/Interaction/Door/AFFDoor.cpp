// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Interaction/Door/AFFDoor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Characters/Firefighters/FirefighterBase.h"
#include "Interfaces/IInteractor.h"

AFFDoor::AFFDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	// DoorMesh 作为根：Actor 变换即门网格；站定点/中轴标记挂它下面
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 纯表现：不阻挡

	// 破碎残骸视觉替身：初始隐藏，破门后替换门本体
	BrokenVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BrokenVisual"));
	BrokenVisual->SetupAttachment(DoorMesh);
	BrokenVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BrokenVisual->SetHiddenInGame(true);

	// 中轴/合页标记（逻辑朝向参考点，默认在网格原点，BP 可按需挪）
	CenterPivot = CreateDefaultSubobject<USceneComponent>(TEXT("CenterPivot"));
	CenterPivot->SetupAttachment(DoorMesh);

	FrontStandPoint = CreateDefaultSubobject<USceneComponent>(TEXT("FrontStandPoint"));
	FrontStandPoint->SetupAttachment(DoorMesh);

	BackStandPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BackStandPoint"));
	BackStandPoint->SetupAttachment(DoorMesh);

	// 把前后站定点注册为基类"操作点"
	RegisterActionPoint(FrontStandPoint);
	RegisterActionPoint(BackStandPoint);

	// 门默认初始为关（标签与插件门 ABP 一致）
	ClosedStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Interaction.Door.Closed")));
	OpenStateTag   = FGameplayTag::RequestGameplayTag(FName(TEXT("Interaction.Door.Open")));
	InitialState   = ClosedStateTag;
}


void AFFDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bShowStandPoints)
	{
		return;
	}

	// PIE/游戏里显示三个锚点：绿=前站定点、红=后站定点、白=中轴
	if (FrontStandPoint)
	{
		DrawDebugSphere(GetWorld(), FrontStandPoint->GetComponentLocation(), 6.f, 12, FColor::Green, false, -1.f, 0, 2.f);
	}
	if (BackStandPoint)
	{
		DrawDebugSphere(GetWorld(), BackStandPoint->GetComponentLocation(), 6.f, 12, FColor::Red, false, -1.f, 0, 2.f);
	}
	if (CenterPivot)
	{
		DrawDebugSphere(GetWorld(), CenterPivot->GetComponentLocation(), 6.f, 12, FColor::White, false, -1.f, 0, 2.f);
	}
}


bool AFFDoor::IsInteractorInFront(AActor* Interactor) const
{
	if (!Interactor)
	{
		return true;
	}

	FVector Dir2D = (Interactor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	return FVector::DotProduct(GetActorForwardVector().GetUnsafeNormal2D(), Dir2D) >= 0.f;
}


USceneComponent* AFFDoor::SelectActionPoint(AActor* User) const
{
	return IsInteractorInFront(User) ? FrontStandPoint : BackStandPoint;
}


void AFFDoor::BeginPlay()
{
	Super::BeginPlay(); // 会把状态置为 InitialState(Closed) 并广播

	// 开局按当前状态设碰撞（关=挡，开=可过）
	SetDoorCollision(true, GetState() == OpenStateTag);
}


void AFFDoor::SetDoorCollision(bool bEnable, bool bEndsOpen)
{
	if (!DoorMesh)
	{
		return;
	}

	if (!bEnable)
	{
		// 开关门动画期间：完全无碰撞（不会被门推/夹到角色）
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	// 开着的门：只保留检测（不挡通行）
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DoorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DoorMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2 /*FireFighter*/, ECR_Overlap);

	if (!bEndsOpen && bBlockWhenClosed)
	{
		// 关着的门：物理阻挡 + 保留 CanInteractor 检测
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DoorMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		DoorMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2 /*FireFighter*/, ECR_Block);
	}
}


void AFFDoor::OnDoorAnimCollisionRestore(bool bEndsOpen)
{
	SetDoorCollision(true, bEndsOpen);
}


bool AFFDoor::CanUnlock(AActor* User) const
{
	if (const AFirefighterBase* FF = Cast<AFirefighterBase>(User))
	{
		return FF->HasKey();
	}
	return false;
}


void AFFDoor::Interact(AActor* Interactor)
{
	// 朝向已由站定点(GetInteractionStand)在到达时定好，这里不再自行转向

	const bool bOpening = (GetState() == ClosedStateTag);

	// 关着 + 锁着 + 有钥匙 → 两段式：插钥匙解锁 → 停0.2s → 普通开门
	if (bOpening && bLocked && CanUnlock(Interactor))
	{
		StartUnlockOpen(Interactor);
		return;
	}

	// 关着 + 锁着 + 没钥匙 → 手持斧头可破门，否则只播"打不开"动画
	if (bOpening && bLocked)
	{
		// 破门条件：交互者手持可破门工具（斧头）
		bool bCanBreak = false;
		if (const AFirefighterBase* FF = Cast<AFirefighterBase>(Interactor))
		{
			bCanBreak = FF->CanBreakDoor();
		}

		if (bCanBreak)
		{
			BreakDoor(Interactor);
			return;
		}

		UAnimMontage* FailMontage = IsInteractorInFront(Interactor) ? FailOpenMontage_Front : FailOpenMontage_Back;
		PlayMontageOnUser(Interactor, FailMontage);
		return; // 不 SetState、不动碰撞
	}

	// 普通开/关 + 前(推)/后(拉)
	const bool bFront = IsInteractorInFront(Interactor);
	UAnimMontage* Montage = nullptr;
	if (bOpening)
	{
		Montage = bFront ? OpenPushMontage : OpenPullMontage;
	}
	else
	{
		Montage = bFront ? ClosePushMontage : ClosePullMontage;
	}
	PlayMontageOnUser(Interactor, Montage);

	// 动画期间门无碰撞、结束后恢复
	const bool bEndsOpen = bOpening;
	if (bEndsOpen)
	{
		bLocked = false;
	}

	SetDoorCollision(false, bEndsOpen);

	GetWorldTimerManager().ClearTimer(CollisionRestoreTimer);
	const float Dur = Montage ? Montage->GetPlayLength() : DoorAnimDuration;
	GetWorldTimerManager().SetTimer(CollisionRestoreTimer,
		FTimerDelegate::CreateUObject(this, &AFFDoor::OnDoorAnimCollisionRestore, bEndsOpen),
		FMath::Max(Dur, 0.05f), false);

	SetState(bEndsOpen ? OpenStateTag : ClosedStateTag);
}


void AFFDoor::PlayMontageOnUser(AActor* User, UAnimMontage* Montage)
{
	if (IInteractor* Unit = Cast<IInteractor>(User))
	{
		Unit->PlayActionMontage(Montage); // 内部：Montage 为空则忽略，不置忙
	}
	else if (ACharacter* Char = Cast<ACharacter>(User))
	{
		if (Montage)
		{
			Char->PlayAnimMontage(Montage);
		}
	}
}


void AFFDoor::StartUnlockOpen(AActor* User)
{
	StagedUser = User;

	// 1) 播"插钥匙解锁"动画（前/后）
	UAnimMontage* UnlockMontage = IsInteractorInFront(User) ? UnlockOpenMontage_Front : UnlockOpenMontage_Back;
	PlayMontageOnUser(User, UnlockMontage);

	// 全程门无碰撞，直到最终打开
	SetDoorCollision(false, true);

	GetWorldTimerManager().ClearTimer(CollisionRestoreTimer);
	const float Dur = UnlockMontage ? UnlockMontage->GetPlayLength() : DoorAnimDuration;
	GetWorldTimerManager().SetTimer(CollisionRestoreTimer,
		FTimerDelegate::CreateUObject(this, &AFFDoor::StagePlayOpenMontage),
		FMath::Max(Dur, 0.05f) + 0.2f, false); // +0.2s 停顿
}


void AFFDoor::StagePlayOpenMontage()
{
	// 2) 解锁完成，门此刻与"普通开门动作"一起转开
	bLocked = false;
	SetState(OpenStateTag);

	AActor* User = StagedUser.Get();
	UAnimMontage* OpenMontage = IsInteractorInFront(User) ? OpenPushMontage : OpenPullMontage;
	PlayMontageOnUser(User, OpenMontage);

	GetWorldTimerManager().ClearTimer(CollisionRestoreTimer);
	const float Dur = OpenMontage ? OpenMontage->GetPlayLength() : DoorAnimDuration;
	GetWorldTimerManager().SetTimer(CollisionRestoreTimer,
		FTimerDelegate::CreateUObject(this, &AFFDoor::StageFinishOpen),
		FMath::Max(Dur, 0.05f), false);
}


void AFFDoor::StageFinishOpen()
{
	// 3) 开门动画结束：恢复碰撞（门已 Open）
	StagedUser = nullptr;
	SetDoorCollision(true, true);
}


void AFFDoor::BreakDoor(AActor* User)
{
	// 消耗破门体力（斧头）
	if (AFirefighterBase* FF = Cast<AFirefighterBase>(User))
	{
		FF->ApplyBreakDoorStaminaCost();
	}

	// 1) 播劈砍动画（共用单段）
	PlayMontageOnUser(User, ChopMontage);

	// 动画期间门无碰撞
	SetDoorCollision(false, true);

	// 2) 定时到劈砍结束 → 隐藏门本体、显示破碎残骸
	GetWorldTimerManager().ClearTimer(CollisionRestoreTimer);
	const float Dur = ChopMontage ? ChopMontage->GetPlayLength() : DoorAnimDuration;
	GetWorldTimerManager().SetTimer(CollisionRestoreTimer,
		FTimerDelegate::CreateUObject(this, &AFFDoor::FinishBreakDoor),
		FMath::Max(Dur, 0.05f), false);
}


void AFFDoor::FinishBreakDoor()
{
	// 门已破碎：本体隐藏、彻底无碰撞（可通行）；显示破碎残骸视觉替身
	DoorMesh->SetVisibility(false);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (BrokenVisual)
	{
		if (BrokenMeshAsset)
		{
			BrokenVisual->SetStaticMesh(BrokenMeshAsset);
		}
		BrokenVisual->SetHiddenInGame(false);
	}

	bLocked = false;
}
