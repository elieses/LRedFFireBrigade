// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ISelectable.h"
#include "Interfaces/IMoveable.h"
#include "Interfaces/IInteractor.h"
#include "Interfaces/IFFItemUser.h"
#include "FirefighterBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AFFInteractableActor;
class UAnimMontage;
class UFFEquipmentComponent;
class UFFCharacterStatsComponent;
class UFFCharacterData;
struct FHitResult;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_UnEquipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName="EquippedOneHandedWeapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName="EquippedTwoHandedWeapon")
};

// 移动模式：走路 / 奔跑 / 蹲下（Z 切走跑，X 切蹲站）
UENUM(BlueprintType)
enum class EFFMovementState : uint8
{
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Crouch UMETA(DisplayName = "Crouch")
};

UCLASS()
class LREDFFIREBRIGADE_API AFirefighterBase : public ACharacter, public ISelectable, public IMoveable, public IInteractor, public IFFItemUser
{
	GENERATED_BODY()

public:
	AFirefighterBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	virtual void BeginPlay() override;

public:
	// 选中指示：脚下圆环
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
	UStaticMeshComponent* SelectionMesh;

	virtual void SetSelected(bool bSelected) override;
	virtual void MoveToLocation(const FVector& Destination) override;
	virtual void TryInteract() override;
	virtual void CommandInteract(AActor* Target) override;
	virtual void TryPickup() override;
	virtual void PlayActionMontage(UAnimMontage* Montage) override;
	virtual UFFEquipmentComponent* GetEquipment() const override { return Equipment; }

	// 当前持握状态（供动画蓝图/蓝图读取）
	UFUNCTION(BlueprintPure, Category = "Equipment|State")
	ECharacterState GetCharacterState() const { return CharacterState; }

	// 切换到指定手持槽（键盘 1 / 点击格子）
	UFUNCTION(BlueprintCallable, Category = "Equipment|State")
	void SwitchToHandSlot(int32 Index);

	// 切换到空手（键盘 G / 点击空手格）
	UFUNCTION(BlueprintCallable, Category = "Equipment|State")
	void SwitchToEmptyHand();

	// 当前移动模式（供动画蓝图/蓝图读取）
	UFUNCTION(BlueprintPure, Category = "Movement|State")
	EFFMovementState GetMovementState() const;

	// 当前手持物品是否能劈砍破门（斧头）
	UFUNCTION(BlueprintPure, Category = "Equipment|State")
	bool CanBreakDoor() const;

	// 劈砍破门消耗体力（由门调用）
	UFUNCTION(BlueprintCallable, Category = "Equipment|State")
	void ApplyBreakDoorStaminaCost();

	// Z：切换走路/奔跑（蹲下时也切换记忆的 base 模式，站起后生效）
	UFUNCTION(BlueprintCallable, Category = "Movement|State")
	void ToggleWalkRun();

	// X：蹲下/站立切换
	UFUNCTION(BlueprintCallable, Category = "Movement|State")
	void ToggleCrouchState();

	// 是否蹲下
	UFUNCTION(BlueprintPure, Category = "Movement|State")
	bool IsCrouched() const { return bCrouched; }

	// 是否体力力竭（力竭期间不能奔跑）
	UFUNCTION(BlueprintPure, Category = "Movement|State")
	bool IsStaminaExhausted() const { return bStaminaExhausted; }

	// 手持装备挂载的骨骼插槽名（编辑器可改）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Visual")
	FName HeldSocketName = TEXT("Eslot");

	// 手持装备的显示网格（切换到装备时挂到手部插槽）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	UStaticMeshComponent* HeldMeshVisual;

	// 是否正忙（在播放交互动作，移动/新交互会被暂存）
	bool IsBusy() const { return bIsBusy; }

	// 显示球体: 笼罩范围内的 IFogRevealable 物体会显示, 离开后隐藏
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FogReveal")
	USphereComponent* RevealSphere;
	// 显示范围 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FogReveal")
	float RevealRange = 800.f;

	// 交互检测距离 (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionRadius = 300.f;

	// 临时：是否带钥匙（锁门判定用，后续换成背包）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	bool bHasKey = false;

	bool HasKey() const { return bHasKey; }

	// 装备组件（物品集合）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment")
	UFFEquipmentComponent* Equipment;

	// 状态组件（血量/体力）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	UFFCharacterStatsComponent* Stats;

	// 角色数据资产（头像/名称/属性上限）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	UFFCharacterData* CharacterData;

protected:
	// 球体重叠回调
	UFUNCTION()
	void OnRevealSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnRevealSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// 处理开局已处于球体内的物体 (重叠事件可能不触发)
	void ScanInitialReveals();

	// 取消"走位后交互"的待定命令
	void CancelPendingInteract();

	// 待交互目标（右键指定）
	TWeakObjectPtr<AFFInteractableActor> PendingInteractTarget;

	// 走到"站定点"后再交互（物体实现了 GetInteractionStand 时用）
	bool bPreciseStand = false;
	FVector PendingStandLocation = FVector::ZeroVector;
	FRotator PendingStandRotation = FRotator::ZeroRotator;

	// 站定点到达判定阈值（cm）：到门口点位这么近就触发交互
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float StandArriveThreshold = 25.f;

	// 调试：运行时显示角色正面朝向箭头
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
	bool bShowFacing = true;

private:
	// 忙时暂存的单条命令（最后一条覆盖前面）
	enum class EFFPendingType : uint8 { Move, InteractObject, InteractNearest, PickupNearest };

	struct FFFPendingCommand
	{
		EFFPendingType Type;
		FVector Dest = FVector::ZeroVector;
		TWeakObjectPtr<AFFInteractableActor> Target;

		FFFPendingCommand() : Type(EFFPendingType::Move) {}
	};

	// 动作动画结束回调：解除忙并执行暂存命令
	UFUNCTION()
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 装备组件变化回调：重算持握状态
	UFUNCTION()
	void OnEquipmentChanged();

	void DoMoveTo(const FVector& Destination);
	void DoCommandInteract(AActor* Target);
	void DoTryInteract();
	void DoTryPickup();
	void ExecutePendingCommand();

	// 根据当前激活手持物品的 GripTag 刷新 CharacterState
	void UpdateCharacterState();

	// 根据当前激活手持物品刷新手部挂载网格（空手则隐藏）
	void UpdateHeldMeshVisual();

	// 按当前移动状态刷新 MaxWalkSpeed（跑/走/蹲）
	void UpdateMovementSpeed();

	// 记忆的 base 移动模式（走路/奔跑；蹲下只是叠加态，不改 base）
	UPROPERTY()
	EFFMovementState BaseMovementState = EFFMovementState::Walk;

	bool bCrouched = false;

	// 由 CharacterData 缓存的速度
	float CachedWalkSpeed = 140.f;
	float CachedRunSpeed = 300.f;
	float CachedCrouchSpeed = 80.f;

	// 由 CharacterData 缓存的体力速率
	float CachedRunStaminaDrain = 10.f;
	float CachedWalkStaminaRegen = 5.f;
	float CachedCrouchStaminaRegen = 10.f;
	float CachedIdleStaminaRegen = 10.f;
	float CachedRunStaminaResume = 40.f;

	// 体力是否已力竭（=0 后置 true，回复到阈值后置 false）
	bool bStaminaExhausted = false;

	// 判定"是否在移动"的速度阈值(cm/s)
	static constexpr float MoveSpeedThreshold = 10.f;

	bool bIsBusy = false;
	TOptional<FFFPendingCommand> PendingCommand;
	
	ECharacterState CharacterState=ECharacterState::ECS_UnEquipped;
};
