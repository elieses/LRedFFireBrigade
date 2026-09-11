// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class ISelectable;
class IMoveable;
class AMoveIndicator;
class AActor;
class UFFBackpackWidget;
/**
 * 
 */
UCLASS()
class LREDFFIREBRIGADE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void HandleMove(const FInputActionValue& Value);
	void HandleZoom(const FInputActionValue& Value);
	void HandleRotate(const FInputActionValue& Value);
	void HandleSelect(const FInputActionValue& Value);
	void HandleMoveCommand(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);
	void HandlePickup(const FInputActionValue& Value);
	void HandleSwitchHand1(const FInputActionValue& Value);
	void HandleSwitchEmptyHand(const FInputActionValue& Value);
	void HandleToggleWalkRun(const FInputActionValue& Value);
	void HandleToggleCrouch(const FInputActionValue& Value);
private:
	// IMC对应模式输入方案
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* IMC_Default;
	// Input Actions
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Move;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Zoom;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Rotate;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Click;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_RightClick;

	// 交互键 (E)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Interact;

	// 拾取键 (F)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Pickup;

	// 切到第一个手持装备格 (1)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_SwitchHand1;

	// 切到空手 (G)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_SwitchEmptyHand;

	// 切换走路/奔跑 (Z)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_ToggleWalkRun;

	// 蹲下/站立 (X)
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_ToggleCrouch;

	// 右键移动指示器的蓝图类（在编辑器里指定）
	UPROPERTY(EditAnywhere, Category="Movement")
	TSubclassOf<AMoveIndicator> MoveIndicatorClass;

	// 背包 Widget（编辑器里指定 WBP_Backpack）
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFFBackpackWidget> BackpackWidgetClass;

	// 当前背包 Widget 实例
	UPROPERTY()
	TObjectPtr<UFFBackpackWidget> BackpackWidget;

	// 当前选中的对象（弱引用，避免悬垂）
	TWeakObjectPtr<AActor> SelectedActor;
	void SetSelectedActor(AActor* NewActor);

	// 根据当前选中刷新背包显示/绑定
	void UpdateBackpackForSelection();

	// 当前活动的移动指示器（新指令时销毁旧的）
	TWeakObjectPtr<AMoveIndicator> ActiveIndicator;
};
