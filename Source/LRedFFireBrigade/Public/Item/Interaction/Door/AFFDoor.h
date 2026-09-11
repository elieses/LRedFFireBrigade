// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/Interaction/AFFInteractableActor.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "AFFDoor.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UAnimMontage;
class UStaticMesh;

// 门（纯表现）：骨骼门 Door_Rig + ABP_Door_Rig_Skeleton，状态 Closed/Open 切换驱动门动画。
// 门 BP 需把本类状态同步给 ABP 的 CurrentState 变量（OnStateChanged 接线）。
UCLASS()
class LREDFFIREBRIGADE_API AFFDoor : public AFFInteractableActor
{
	GENERATED_BODY()

public:
	AFFDoor();

	virtual void Tick(float DeltaTime) override;

	// 交互：开/关切换
	virtual void Interact(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

	// 开关门动画期间把门设为无碰撞；结束后按最终状态恢复
	void SetDoorCollision(bool bEnable, bool bEndsOpen);

	// 动画结束后恢复碰撞（定时回调）
	void OnDoorAnimCollisionRestore(bool bEndsOpen);

	UPROPERTY(EditAnywhere, Category = "Interaction|Door", meta = (ClampMin = 0.f))
	float DoorAnimDuration = 0.6f;   // 无蒙太奇时的门动画时长兜底

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	bool bBlockWhenClosed = true;    // 关着的门是否物理阻挡

	FTimerHandle CollisionRestoreTimer;

	UFUNCTION(BlueprintPure, Category = "Interaction|Door")
	USkeletalMeshComponent* GetDoorMesh() const { return DoorMesh; }

	// 交互者是否在门的前向(+X)一侧（约定 +X 侧 = 推侧）
	bool IsInteractorInFront(AActor* Interactor) const;

	// 按使用者在门前/门后选操作点
	virtual USceneComponent* SelectActionPoint(AActor* User) const override;

	// 能否开锁（以后接背包在这里查 Equipment.Key）
	virtual bool CanUnlock(AActor* User) const;

protected:
	// 运行时用小球显示三个锚点位置（调试/摆放）
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	bool bShowStandPoints = true;

	// 门的中轴/合页锚点（BP 摆放）；门网格与站定点都挂在它下面
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Door")
	USceneComponent* CenterPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Door")
	USceneComponent* FrontStandPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Door")
	USceneComponent* BackStandPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Door")
	USkeletalMeshComponent* DoorMesh;

	// 关/开对应的状态 Tag（默认插件门 ABP 用的标签，编辑器可改）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Door", meta = (Categories = "Interaction"))
	FGameplayTag ClosedStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Door", meta = (Categories = "Interaction"))
	FGameplayTag OpenStateTag;

	// 角色开门/关门动画：按"在前(推)/在后(拉)"分四种，门 ABP 固定方向摆
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* OpenPushMontage;

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* OpenPullMontage;

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* ClosePushMontage;

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* ClosePullMontage;

	// 门是否上锁（关着且没钥匙时只能播打不开动画）
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	bool bLocked = false;

	// 打不开(锁)时角色动作：按前(推侧)/后(拉侧)分开
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* FailOpenMontage_Front;

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* FailOpenMontage_Back;

	// 用钥匙开门时的"插钥匙解锁"动画（只解锁；随后 0.2s 再接普通开门动作）
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* UnlockOpenMontage_Front;

	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* UnlockOpenMontage_Back;

	// 手持斧头破门：单段劈砍动画（共用）
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UAnimMontage* ChopMontage;

	// 破门后的破碎残骸网格（BP 指定；替换门本体显示）
	UPROPERTY(EditAnywhere, Category = "Interaction|Door")
	UStaticMesh* BrokenMeshAsset;

	// 破碎残骸显示组件（初始隐藏，破门后显示）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|Door")
	UStaticMeshComponent* BrokenVisual;

protected:
	// 两段式钥匙开门：解锁动画 → 停0.2s → 普通开门 → 置Open
	void StartUnlockOpen(AActor* User);
	void StagePlayOpenMontage();
	void StageFinishOpen();
	void PlayMontageOnUser(AActor* User, UAnimMontage* Montage);

	// 劈砍破门：播动画 → 定时结束后隐藏门本体、显示破碎残骸
	void BreakDoor(AActor* User);

	// 破门动画结束回调：切换破碎残骸
	void FinishBreakDoor();

	TWeakObjectPtr<AActor> StagedUser;
};
