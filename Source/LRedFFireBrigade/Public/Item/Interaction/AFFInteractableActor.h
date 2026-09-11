// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "AFFInteractableActor.generated.h"

class USceneComponent;

// 可交互物体状态变更事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFFOnInteractableStateChanged, FGameplayTag, NewState);

// 可交互物体基类：自带一个 GameplayTag 状态 + "操作点(ActionPoint)"框架。
// 操作点决定"使用者站在哪、朝哪个方向交互"；子类注册操作点并可选覆写选择规则。
UCLASS()
class LREDFFIREBRIGADE_API AFFInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	AFFInteractableActor();

	// 初始状态（编辑器里设，如 Interaction.Switcher.Off）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (Categories = "Interaction"))
	FGameplayTag InitialState;

	// 状态变更事件（SetState 且值变化时广播）
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FFFOnInteractableStateChanged OnStateChanged;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FGameplayTag GetState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetState(const FGameplayTag& NewState);

	// 交互入口（右键/E 触发），传入操作者，子类实现具体行为
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact(AActor* Interactor);

	// 是否为"仅拾取"型物体（F 专用）；门/开关等通用交互返回 false
	virtual bool IsPickup() const { return false; }

	// 给一个精确"站位 + 朝向"：由"选操作点 + 该点朝向"决定（没有操作点则返回 false）
	virtual bool GetInteractionStand(AActor* User, FVector& OutLocation, FRotator& OutRotation) const;

protected:
	virtual void BeginPlay() override;

	// 注册一个操作点子组件（子类在创建好组件后调用）
	void RegisterActionPoint(USceneComponent* Point);

	// 虚函数：为使用者选择操作点（默认第一个；Door 重写为按前/后选）
	virtual USceneComponent* SelectActionPoint(AActor* User) const;

	// 虚函数：该操作点的朝向（默认取点组件自身朝向，yaw）
	virtual FRotator GetActionFacing(USceneComponent* Point, AActor* User) const;

	UPROPERTY()
	TArray<TWeakObjectPtr<USceneComponent>> ActionPoints;

	UPROPERTY()
	FGameplayTag CurrentState;
};
