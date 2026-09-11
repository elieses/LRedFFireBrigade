// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Firefighters/FirefighterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Interaction/AFFInteractableActor.h"
#include "Item/WorldItem/AFFWorldItem.h"
#include "Item/Equipment/UFFEquipmentComponent.h"
#include "Item/Items/UFFItem.h"
#include "Item/Data/UFFItemData.h"
#include "Characters/Firefighters/UFFCharacterStatsComponent.h"
#include "Characters/Firefighters/UFFCharacterData.h"
#include "AIController.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "Interfaces/IFogRevealable.h"

// FireFighter = ECC_GameTraceChannel2, FogReveal = ECC_GameTraceChannel4,
// CanInteractor = ECC_GameTraceChannel5 (DefaultEngine.ini)
namespace
{
	const ECollisionChannel FireFighterChannel = ECC_GameTraceChannel2;
	const ECollisionChannel FogRevealChannel = ECC_GameTraceChannel4;
	const ECollisionChannel CanInteractorChannel = ECC_GameTraceChannel5;
}

AFirefighterBase::AFirefighterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 使用 AI 控制器，以便接收 MoveToLocation 导航移动命令
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 选中指示：脚下扁圆环（StaticMesh 留空，在编辑器里指定）
	SelectionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectionMesh"));
	SelectionMesh->SetupAttachment(RootComponent);
	SelectionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionMesh->SetHiddenInGame(true);
	SelectionMesh->SetRelativeLocation(FVector(0, 0, 10.f));
	SelectionMesh->SetRelativeScale3D(FVector(1, 1, 0.05f));

	// 显示球体：只与 FogReveal 通道重叠 (要隐藏的物体实现该通道)
	RevealSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RevealSphere"));
	RevealSphere->SetupAttachment(RootComponent);
	RevealSphere->SetSphereRadius(RevealRange);
	RevealSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RevealSphere->SetCollisionObjectType(FireFighterChannel);
	RevealSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	RevealSphere->SetCollisionResponseToChannel(FogRevealChannel, ECR_Overlap);
	RevealSphere->SetGenerateOverlapEvents(true);

	Equipment = CreateDefaultSubobject<UFFEquipmentComponent>(TEXT("Equipment"));

	Stats = CreateDefaultSubobject<UFFCharacterStatsComponent>(TEXT("Stats"));

	// 手持装备显示网格：挂在角色手上，切换装备时更新，初始隐藏
	HeldMeshVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldMeshVisual"));
	HeldMeshVisual->SetupAttachment(GetMesh());
	HeldMeshVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeldMeshVisual->SetHiddenInGame(true);
}


void AFirefighterBase::BeginPlay()
{
	Super::BeginPlay();

	if (RevealSphere)
	{
		RevealSphere->SetSphereRadius(RevealRange);
		RevealSphere->OnComponentBeginOverlap.AddDynamic(this, &AFirefighterBase::OnRevealSphereOverlap);
		RevealSphere->OnComponentEndOverlap.AddDynamic(this, &AFirefighterBase::OnRevealSphereEndOverlap);
		// 先扫一次, 再等一帧补一次 (覆盖出生先后顺序导致的漏判)
		ScanInitialReveals();
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &AFirefighterBase::ScanInitialReveals));
	}

	// 订阅装备组件变化：拾取/替换丢旧/切换手持槽 → 重算持握状态
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.AddDynamic(this, &AFirefighterBase::OnEquipmentChanged);
		UpdateCharacterState();
		UpdateHeldMeshVisual();
	}

	// 从角色数据资产套用属性上限并满值初始化
	if (Stats && CharacterData)
	{
		Stats->SetMaxHealth(CharacterData->MaxHealth);
		Stats->SetMaxStamina(CharacterData->MaxStamina);
		Stats->SetHealth(CharacterData->MaxHealth);
		Stats->SetStamina(CharacterData->MaxStamina);
	}

	// 从角色数据资产套用移动速度并初始化当前速度
	if (CharacterData)
	{
		CachedWalkSpeed = CharacterData->WalkSpeed;
		CachedRunSpeed = CharacterData->RunSpeed;
		CachedCrouchSpeed = CharacterData->CrouchSpeed;

		CachedRunStaminaDrain = CharacterData->RunStaminaDrain;
		CachedWalkStaminaRegen = CharacterData->WalkStaminaRegen;
		CachedCrouchStaminaRegen = CharacterData->CrouchStaminaRegen;
		CachedIdleStaminaRegen = CharacterData->IdleStaminaRegen;
		CachedRunStaminaResume = CharacterData->RunStaminaResume;
	}
	UpdateMovementSpeed();
}


void AFirefighterBase::ScanInitialReveals()
{
	if (!RevealSphere)
	{
		return;
	}
	TArray<AActor*> Overlapping;
	RevealSphere->GetOverlappingActors(Overlapping);
	for (AActor* A : Overlapping)
	{
		if (IFogRevealable* Revealable = Cast<IFogRevealable>(A))
		{
			Revealable->AddRevealSource();
		}
	}
}

void AFirefighterBase::OnRevealSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IFogRevealable* Revealable = Cast<IFogRevealable>(OtherActor))
	{
		Revealable->AddRevealSource();
	}
}

void AFirefighterBase::OnRevealSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IFogRevealable* Revealable = Cast<IFogRevealable>(OtherActor))
	{
		Revealable->RemoveRevealSource();
	}
}


void AFirefighterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 调试：显示角色正面朝向（绿箭头）与站定点目标朝向（黄箭头）
	if (bShowFacing)
	{
		const FVector Start = GetActorLocation() + FVector(0, 0, 90.f);
		DrawDebugDirectionalArrow(GetWorld(), Start, Start + GetActorForwardVector() * 100.f, 12.f, FColor::Green, false, -1.f, 0, 3.f);

		if (bPreciseStand && PendingInteractTarget.IsValid())
		{
			DrawDebugDirectionalArrow(GetWorld(), Start, Start + PendingStandRotation.Vector() * 100.f, 12.f, FColor::Yellow, false, -1.f, 0, 3.f);
		}
	}

	// 走位后交互：到达目标后执行
	if (PendingInteractTarget.IsValid())
	{
		AFFInteractableActor* Target = PendingInteractTarget.Get();

		// 判定依据：站定点模式用"站定点位置 + 小阈值"，否则用"物体位置 + 交互半径"
		const FVector Ref = bPreciseStand ? PendingStandLocation : Target->GetActorLocation();
		const float Reach = bPreciseStand ? StandArriveThreshold : InteractionRadius;

		bool bArrived = (FVector::Dist2D(GetActorLocation(), Ref) <= Reach);

		// 站定点兜底：若 AI 已停但仍差一点没进阈值，且已够近，也算到位
		if (!bArrived && bPreciseStand)
		{
			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle &&
					FVector::Dist2D(GetActorLocation(), Ref) < InteractionRadius)
				{
					bArrived = true;
				}
			}
		}

		if (bArrived)
		{
			const bool bWasPrecise = bPreciseStand;
			PendingInteractTarget = nullptr;
			bPreciseStand = false;

			// 走到站定点后转向门
			if (bWasPrecise)
			{
				if (AAIController* AIController = Cast<AAIController>(GetController()))
				{
					AIController->StopMovement();
				}
				SetActorRotation(PendingStandRotation);
			}
			Target->Interact(this);
		}
	}
	else
	{
		// 目标已失效/被销毁则清掉待定命令
		PendingInteractTarget = nullptr;
		bPreciseStand = false;
	}

	// —— 体力结算：跑-10 / 走+5 / 蹲+10 / 静止+10；交互中(忙)不回复 ——
	if (Stats && !bIsBusy)
	{
		const bool bMoving = GetVelocity().Size2D() > MoveSpeedThreshold;
		float Rate;
		if (!bMoving)
		{
			Rate = CachedIdleStaminaRegen;                 // 静止不动：+10
		}
		else
		{
			switch (GetMovementState())
			{
			case EFFMovementState::Run:
				Rate = -CachedRunStaminaDrain;             // 跑步：-10
				break;
			case EFFMovementState::Crouch:
				Rate = CachedCrouchStaminaRegen;           // 蹲下：+10
				break;
			case EFFMovementState::Walk:
			default:
				Rate = CachedWalkStaminaRegen;             // 行走：+5
				break;
			}
		}
		Stats->ApplyStaminaDelta(Rate * DeltaTime);

		// 力竭判定：空体力 → 临时强制走路（保留 base 意图）；回复到阈值 → 自动恢复奔跑
		const float Cur = Stats->GetStamina();
		if (Cur <= 0.f)
		{
			if (!bStaminaExhausted)
			{
				bStaminaExhausted = true;
				UpdateMovementSpeed();   // 立即降为走速
			}
		}
		else if (Cur >= CachedRunStaminaResume)
		{
			if (bStaminaExhausted)
			{
				bStaminaExhausted = false;
				UpdateMovementSpeed();   // base 若仍是 Run → 自动恢复奔跑
			}
		}
	}
}


void AFirefighterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AFirefighterBase::SetSelected(bool bSelected)
{
	if (SelectionMesh)
	{
		SelectionMesh->SetHiddenInGame(!bSelected);
	}
}


void AFirefighterBase::MoveToLocation(const FVector& Destination)
{
	// 忙：暂存一条命令（覆盖旧暂存）
	if (bIsBusy)
	{
		PendingCommand = FFFPendingCommand();
		PendingCommand->Type = EFFPendingType::Move;
		PendingCommand->Dest = Destination;
		PendingCommand->Target = nullptr;
		return;
	}

	PendingCommand.Reset();
	DoMoveTo(Destination);
}


void AFirefighterBase::DoMoveTo(const FVector& Destination)
{
	// 普通右键移动：取消待交互命令
	CancelPendingInteract();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToLocation(Destination);
	}
}


void AFirefighterBase::CancelPendingInteract()
{
	PendingInteractTarget = nullptr;
	bPreciseStand = false;
}


void AFirefighterBase::CommandInteract(AActor* Target)
{
	AFFInteractableActor* IA = Cast<AFFInteractableActor>(Target);
	if (!IA)
	{
		return;
	}

	// 忙：暂存一条命令（覆盖旧暂存）
	if (bIsBusy)
	{
		PendingCommand = FFFPendingCommand();
		PendingCommand->Type = EFFPendingType::InteractObject;
		PendingCommand->Target = IA;
		return;
	}

	PendingCommand.Reset();
	DoCommandInteract(IA);
}


void AFirefighterBase::DoCommandInteract(AActor* Target)
{
	AFFInteractableActor* IA = Cast<AFFInteractableActor>(Target);
	if (!IA)
	{
		return;
	}

	// 彻底分工：右键不再拾取物品，物品只能由 F(TryPickup) 取
	if (IA->IsPickup())
	{
		return;
	}

	// 物体提供了精确站定点：一律先走到点中心、到位再转向交互（无论远近）
	FVector StandLoc = FVector::ZeroVector;
	FRotator StandRot = FRotator::ZeroRotator;
	if (IA->GetInteractionStand(this, StandLoc, StandRot))
	{
		bPreciseStand = true;
		PendingStandLocation = StandLoc;
		PendingStandRotation = StandRot;
		PendingInteractTarget = IA;

		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->MoveToLocation(StandLoc, 8.f); // 小到达半径，尽量贴点
		}
		return;
	}

	// 无站定点：通用"走到目标附近再交互"
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		return;
	}

	PendingInteractTarget = IA;

	const FVector ToTarget = IA->GetActorLocation() - GetActorLocation();
	const float Dist2D = ToTarget.Size2D();
	const float StopOffset = FMath::Min(Dist2D * 0.5f, InteractionRadius * 0.6f);
	FVector Dest = IA->GetActorLocation() - ToTarget.GetSafeNormal2D() * StopOffset;
	Dest.Z = GetActorLocation().Z;

	AIController->MoveToLocation(Dest);
}


void AFirefighterBase::TryInteract()
{
	// 忙：暂存一条命令（覆盖旧暂存）
	if (bIsBusy)
	{
		PendingCommand = FFFPendingCommand();
		PendingCommand->Type = EFFPendingType::InteractNearest;
		return;
	}

	PendingCommand.Reset();
	DoTryInteract();
}


void AFirefighterBase::DoTryInteract()
{
	// 用 CanInteractor 通道手动球体查询交互半径内最近的可交互物
	// （物体需使用 Interactor 预设：ObjectType=InteractorObject + 响应 CanInteractor=Overlap）
	FCollisionShape Shape = FCollisionShape::MakeSphere(InteractionRadius);
	FCollisionQueryParams QP;
	QP.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, CanInteractorChannel, Shape, QP);

	AFFInteractableActor* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	for (const FOverlapResult& O : Overlaps)
	{
		if (AFFInteractableActor* IA = Cast<AFFInteractableActor>(O.GetActor()))
		{
			if (IA->IsPickup())
			{
				continue; // E 不捡物品，拾取归 F
			}
			const float D = FVector::DistSquared2D(GetActorLocation(), IA->GetActorLocation());
			if (D < BestDistSq)
			{
				BestDistSq = D;
				Nearest = IA;
			}
		}
	}

	if (Nearest)
	{
		// E 也要先到物体的站定点再交互（若有）
		FVector StandLoc = FVector::ZeroVector;
		FRotator StandRot = FRotator::ZeroRotator;
		if (Nearest->GetInteractionStand(this, StandLoc, StandRot))
		{
			bPreciseStand = true;
			PendingStandLocation = StandLoc;
			PendingStandRotation = StandRot;
			PendingInteractTarget = Nearest;

			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				AIController->MoveToLocation(StandLoc, 8.f); // 小到达半径，尽量贴点
			}
			return;
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
				FString::Printf(TEXT("[%s] 交互目标: %s"), *GetName(), *Nearest->GetName()));
		}
		Nearest->Interact(this);
	}
}


void AFirefighterBase::TryPickup()
{
	// 忙：暂存一条命令（覆盖旧暂存）
	if (bIsBusy)
	{
		PendingCommand = FFFPendingCommand();
		PendingCommand->Type = EFFPendingType::PickupNearest;
		return;
	}

	PendingCommand.Reset();
	DoTryPickup();
}


void AFirefighterBase::DoTryPickup()
{
	// 只用 CanInteractor 通道球体查询，但只认 AFFWorldItem（门/开关不是，天然被排除，绝不开门）
	FCollisionShape Shape = FCollisionShape::MakeSphere(InteractionRadius);
	FCollisionQueryParams QP;
	QP.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, CanInteractorChannel, Shape, QP);

	AFFWorldItem* Nearest = nullptr;
	float BestDistSq = FLT_MAX;
	for (const FOverlapResult& O : Overlaps)
	{
		if (AFFWorldItem* Item = Cast<AFFWorldItem>(O.GetActor()))
		{
			if (Item->IsTaken())
			{
				continue; // 已取走的不参与
			}
			const float D = FVector::DistSquared2D(GetActorLocation(), Item->GetActorLocation());
			if (D < BestDistSq)
			{
				BestDistSq = D;
				Nearest = Item;
			}
		}
	}

	if (Nearest)
	{
		Nearest->Interact(this);
	}
}


void AFirefighterBase::PlayActionMontage(UAnimMontage* Montage)
{
	// 仅配置了蒙太奇才"忙"
	if (!Montage)
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	UAnimInstance* Anim = SkeletalMesh ? SkeletalMesh->GetAnimInstance() : nullptr;
	if (!Anim)
	{
		return;
	}

	bIsBusy = true;
	Anim->OnMontageEnded.AddDynamic(this, &AFirefighterBase::OnActionMontageEnded);
	PlayAnimMontage(Montage);
}


void AFirefighterBase::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsBusy = false;

	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (UAnimInstance* Anim = SkeletalMesh->GetAnimInstance())
		{
			Anim->OnMontageEnded.RemoveDynamic(this, &AFirefighterBase::OnActionMontageEnded);
		}
	}

	// 忙时暂存的那条命令现在执行
	ExecutePendingCommand();
}


void AFirefighterBase::ExecutePendingCommand()
{
	if (!PendingCommand.IsSet())
	{
		return;
	}

	FFFPendingCommand Cmd = PendingCommand.GetValue();
	PendingCommand.Reset();

	switch (Cmd.Type)
	{
	case EFFPendingType::Move:
		DoMoveTo(Cmd.Dest);
		break;
	case EFFPendingType::InteractObject:
		if (Cmd.Target.IsValid())
		{
			DoCommandInteract(Cmd.Target.Get());
		}
		break;
	case EFFPendingType::InteractNearest:
		DoTryInteract();
		break;
	case EFFPendingType::PickupNearest:
		DoTryPickup();
		break;
	}
}


void AFirefighterBase::SwitchToHandSlot(int32 Index)
{
	if (Equipment)
	{
		Equipment->SelectHandSlot(Index);
	}
}


void AFirefighterBase::SwitchToEmptyHand()
{
	if (Equipment)
	{
		Equipment->SelectEmptyHands();
	}
}


void AFirefighterBase::OnEquipmentChanged()
{
	UpdateCharacterState();
	UpdateHeldMeshVisual();
}


void AFirefighterBase::UpdateCharacterState()
{
	UFFItem* Active = Equipment ? Equipment->GetActiveHandItem() : nullptr;
	if (!Active)
	{
		CharacterState = ECharacterState::ECS_UnEquipped;
		return;
	}

	if (Active->ItemDataRef && Active->ItemDataRef->GripTag == FGameplayTag::RequestGameplayTag(FName(TEXT("Equipment.Grip.TwoHand"))))
	{
		CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
	}
	else
	{
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	}
}


void AFirefighterBase::UpdateHeldMeshVisual()
{
	UFFItem* Active = Equipment ? Equipment->GetActiveHandItem() : nullptr;
	UStaticMesh* MeshToShow = (Active && Active->ItemDataRef) ? Active->ItemDataRef->EquipmentMesh : nullptr;

	if (!HeldMeshVisual)
	{
		return;
	}

	if (!MeshToShow)
	{
		HeldMeshVisual->SetHiddenInGame(true);
		HeldMeshVisual->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		return;
	}

	// 设置网格并挂到角色的手部插槽（socket 相对变换由骨骼编辑器调）
	if (HeldMeshVisual->GetStaticMesh() != MeshToShow)
	{
		HeldMeshVisual->SetStaticMesh(MeshToShow);
	}
	HeldMeshVisual->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeldSocketName);
	HeldMeshVisual->SetHiddenInGame(false);
}


EFFMovementState AFirefighterBase::GetMovementState() const
{
	// 蹲下是叠加态；力竭则临时强制走路（base 保留玩家意图，恢复后自动回奔跑）
	if (bCrouched)
	{
		return EFFMovementState::Crouch;
	}
	if (bStaminaExhausted)
	{
		return EFFMovementState::Walk;
	}
	return BaseMovementState;
}

bool AFirefighterBase::CanBreakDoor() const
{
	UFFItem* Active = Equipment ? Equipment->GetActiveHandItem() : nullptr;
	return Active && Active->ItemDataRef && Active->ItemDataRef->bCanBreakDoor;
}

void AFirefighterBase::ApplyBreakDoorStaminaCost()
{
	UFFItem* Active = Equipment ? Equipment->GetActiveHandItem() : nullptr;
	if (Stats && Active && Active->ItemDataRef)
	{
		Stats->ApplyStaminaDelta(-Active->ItemDataRef->BreakDoorStaminaCost);
	}
}

void AFirefighterBase::ToggleWalkRun()
{
	// 切换玩家的 base 意图（力竭只是临时覆盖：意图为 Run 时，体力恢复后自动继续奔跑；
	// 力竭期间玩家按 Z 可把意图改成 Walk，从而取消自动恢复）
	BaseMovementState = (BaseMovementState == EFFMovementState::Walk) ? EFFMovementState::Run : EFFMovementState::Walk;

	if (!bCrouched)
	{
		UpdateMovementSpeed();
	}
}

void AFirefighterBase::ToggleCrouchState()
{
	bCrouched = !bCrouched;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (bCrouched)
		{
			MoveComp->Crouch();
		}
		else
		{
			MoveComp->UnCrouch();
		}
	}

	UpdateMovementSpeed();
}

void AFirefighterBase::UpdateMovementSpeed()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		switch (GetMovementState())
		{
		case EFFMovementState::Run:
			MoveComp->MaxWalkSpeed = CachedRunSpeed;
			break;
		case EFFMovementState::Crouch:
			MoveComp->MaxWalkSpeed = CachedCrouchSpeed;
			break;
		case EFFMovementState::Walk:
		default:
			MoveComp->MaxWalkSpeed = CachedWalkSpeed;
			break;
		}
	}
}

